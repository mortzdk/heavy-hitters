#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <criterion/criterion.h>

#include "util/hash.h"
#include "sketch/count_median.h"
#include "sketch/sketch.h"
#include "norm/norm.h"
#include "norm/l2-sketch.h"
#include "util/xutil.h"

#define INSERTS 8192

Test(norm_l2_sketch, working_create_and_destroy, .disabled=0) {
	uint8_t b         = 6;
	double  epsilon   = 0.25;
	double  delta     = 0.20;

	l2_sketch_config cfg = {
		.sketch = sketch_create(&countMedian, &carterWegman, b, epsilon, delta),
	};

	norm_t *norm = norm_create(&cfg, &norm_func_l2_sketch);

	norm_destroy(norm);
}

Test(norm_l2_sketch, expect_simple_result, .disabled=0) {
	uint8_t b         = 6;
	double  epsilon   = 0.25;
	double  delta     = 0.20;

	l2_sketch_config cfg = {
		.sketch = sketch_create(&countMedian, &carterWegman, b, epsilon, delta),
	};

	norm_t *norm = norm_create(&cfg, &norm_func_l2_sketch);

	norm_update(norm, 12, 100);

	int64_t result = norm_norm(norm);

	cr_assert_eq(result, 100, "Wrong result, got: '%"PRIi64"'", result);

	norm_destroy(norm);
}

Test(norm_l2_sketch, expect_between_l1, .disabled=0) {
	int i;
	int64_t value;
	int64_t L2 = 0;
	int64_t L1 = 0;
	double sqrtL1;

	// Sketch params
	uint8_t b         = 6;
	double  epsilon   = 0.25;
	double  delta     = 0.03125;

	l2_sketch_config cfg = {
		.sketch = sketch_create(&countMedian, &carterWegman, b, epsilon, delta),
	};

	norm_t *norm = norm_create(&cfg, &norm_func_l2_sketch);

	for (i = 0; i < INSERTS; i++) {
		value = xuni_rand() * ((int32_t)(1 << 9) -1);
		norm_update(norm, i, value);

		L2 += value * value;
		L1 += value;
	}

	int64_t result = norm_norm(norm);

	L2 = (int64_t) sqrt(L2);
	sqrtL1 = sqrt(L1);

	cr_assert_leq(result, L1, "Not smaller than L1 norm, L1: %"PRIi64", res: %"PRIi64"", L1, result);

	cr_assert_geq(result, sqrtL1, "Not larger than sqrt(L1 norm), sqrt(L1): %f, res: %"PRIi64"", sqrtL1, result);

	//cr_assert_eq(result, L2, "Not equal to actual L2 norm, L2: %"PRIi64", res: %"PRIi64"", L2, result);
	printf("[INFO] norm_l2_sketch::expect_between_l1: L2: %"PRIi64" --- Result: %"PRIi64"\n", L2, result);


	norm_destroy(norm);
}

Test(norm_l2_sketch, expect_between_l1_duplicates, .disabled=0) {
	int i;
	int32_t id;
	int64_t value;
	int64_t L2 = 0;
	int64_t L1 = 0;
	double sqrtL1;

	// Sketch params
	uint8_t b         = 6;
	double  epsilon   = 0.25;
	double  delta     = 0.03125;

	l2_sketch_config cfg = {
		.sketch = sketch_create(&countMedian, &carterWegman, b, epsilon, delta),
	};

	int unique = sketch_width(cfg.sketch->sketch) * 16;

	norm_t *norm = norm_create(&cfg, &norm_func_l2_sketch);

	int64_t *values = xmalloc(unique * sizeof(int64_t));

	// Reset values
	for (i = 0; i < unique; i++) {
		values[i] = 0;
	}

	for (i = 0; i < INSERTS; i++) {
		value = xuni_rand() * ((int32_t)(1 << 9) -1);
		id = i % unique;

		norm_update(norm, id, value);
		values[id] += value;

		L1 += value;
	}

	// Calculate L2 norm
	for (i = 0; i < unique; i++) {
		L2 += values[i] * values[i];
	}
	L2 = (int64_t) sqrt(L2);

	int64_t result = norm_norm(norm);

	sqrtL1 = sqrt(L1);

	cr_assert_leq(result, L1, "Not smaller than L1 norm, L1: %"PRIi64", res: %"PRIi64"", L1, result);

	cr_assert_geq(result, sqrtL1, "Not larger than sqrt(L1 norm), sqrt(L1): %f, res: %"PRIi64"", sqrtL1, result);

	//cr_assert_eq(result, L2, "Not equal to actual L2 norm, L2: %"PRIi64", res: %"PRIi64"", L2, result);
	printf("[INFO] norm_l2_sketch::expect_between_l1_duplicates: L2: %"PRIi64" --- Result: %"PRIi64"\n", L2, result);

	free(values);

	norm_destroy(norm);
}
