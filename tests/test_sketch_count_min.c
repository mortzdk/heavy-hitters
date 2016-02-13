#include <stdint.h>
#include <stdio.h>
#include <criterion/criterion.h>

#include "hash.h"
#include "sketch/count_min.h"
#include "sketch/sketch.h"

Test(count_min_sketch, expected_d, .disabled=0) {
	sketch_t *s = sketch_create(&countMin, &hash31, 2, 0.25, 0.2);
	count_min_t *cm = s->sketch;

	cr_assert_eq(cm->size.d, 3, "Wrong d value, expected %d got %"PRIu32, 3, cm->size.d);

	sketch_destroy(s);
}

Test(count_min_sketch, expected_w_16, .disabled=0) {
	sketch_t *s = sketch_create(&countMin, &hash31, 2, 0.25, 0.2);
	count_min_t *cm = s->sketch;

	cr_assert_eq(cm->size.w, 8, "Wrong w value, expected %d got %"PRIu32, 8, cm->size.w);

	sketch_destroy(s);
}

Test(count_min_sketch, update_point, .disabled=0) {
	sketch_t *s = sketch_create(&countMin, &hash31, 2, 0.3, 0.2);

	sketch_update(s, 9, 42);
	uint32_t estimate = sketch_point(s, 9);

	cr_assert_eq(estimate, 42, "Estimate (%d) should be 42", estimate);

	sketch_destroy(s);
}

Test(count_min_sketch, update_point_2, .disabled=0) {
	uint32_t estimate;
	uint8_t b         = 2;
	double  epsilon   = 0.25;
	double  delta     = 0.20;
	uint32_t A[10][2] = {
		{1, 3543},
		{2, 7932},
		{3, 8234},
		{4, 48},
		{5, 58},
		{6, 238},
		{7, 732},
		{8, 10038},
		{9, 78923},
		{327, 78}
	};

	sketch_t *s0 = sketch_create(&countMin, &hash31, b, epsilon, delta);
	sketch_t *s1 = sketch_create(&countMin, &hash31p2, b, epsilon, delta);
	sketch_t *s2 = sketch_create(&countMin, &multiplyShift, b, epsilon, delta);

	for (int i = 0; i < 10; i++) {
		sketch_update(s0, A[i][0], A[i][1]);
		sketch_update(s1, A[i][0], A[i][1]);
		sketch_update(s2, A[i][0], A[i][1]);
	}

	for (int i = 0; i < 10; i++) {
		estimate = sketch_point(s0, A[i][0]);
		cr_expect_geq(estimate, A[i][1], "Estimate (%d) should be %d, i = %d", estimate, A[i][1], i);

		estimate = sketch_point(s1, A[i][0]);
		cr_expect_geq(estimate, A[i][1], "Estimate (%d) should be %d, i = %d", estimate, A[i][1], i);

		estimate = sketch_point(s2, A[i][0]);
		cr_expect_geq(estimate, A[i][1], "Estimate (%d) should be %d, i = %d", estimate, A[i][1], i);
	}

	sketch_destroy(s0);
	sketch_destroy(s1);
	sketch_destroy(s2);
}

Test(count_min_sketch, update_point_3, .disabled=0) {
	uint32_t estimate;
	uint32_t A[10][2] = {
		{42, 3543},
		{42, 7932},
		{42, 8234},
		{42, 48},
		{42, 58},
		{42, 238},
		{42, 732},
		{42, 10038},
		{42, 78923},
		{42, 78}
	};

	sketch_t *s = sketch_create(&countMin, &hash31, 2, 0.3, 0.2);

	for (int i = 0; i < 10; i++) {
		sketch_update(s, A[i][0], A[i][1]);
	}

	uint32_t sum = 0;

	for (int i = 0; i < 10; i++) {
		sum += A[i][1];
	}

	estimate = sketch_range_sum(s, 42, 42);
	cr_assert_eq(estimate, sum, "Estimate (%d) should be %d", estimate, sum);

	estimate = sketch_range_sum(s, 0, 100);
	cr_assert_eq(estimate, sum, "Estimate (%d) should be %d", estimate, sum);

	sketch_destroy(s);
}

Test(count_min_sketch, update_range_naive_1, .disabled=0) {
	uint32_t estimate;
	uint32_t A[10][2] = {
		{42, 3543},
		{42, 7932},
		{42, 8234},
		{42, 48},
		{42, 58},
		{42, 238},
		{42, 732},
		{42, 10038},
		{42, 78923},
		{42, 78}
	};

	sketch_t *s = sketch_create(&countMin, &hash31, 2, 0.3, 0.2);

	for (int i = 0; i < 10; i++) {
		sketch_update(s, A[i][0], A[i][1]);
	}

	uint32_t sum = 0;

	for (int i = 0; i < 10; i++) {
		sum += A[i][1];
	}

	estimate = sketch_point(s, 42);
	cr_assert_eq(estimate, sum, "Estimate (%d) should be %d", estimate, sum);

	sketch_destroy(s);
}

Test(count_min_sketch, should_allocate_internals, .disabled=0) {
	sketch_t *s     = sketch_create(&countMin, &hash31, 2, 0.5, 0.2);
	count_min_t *cm = s->sketch;

	cr_assert_not_null(cm, "Expedted an allocated structure");
	cr_assert_not_null(cm->table, "Expedted an allocated structure");

	sketch_destroy(s);
}

Test(count_min_sketch, should_free_object, .disabled=1) {
	sketch_t *s = sketch_create(&countMin, &hash31, 2, 0.5, 0.2);
	sketch_destroy(s);

	cr_assert_null(s, "Expedted an empty structure");
}
