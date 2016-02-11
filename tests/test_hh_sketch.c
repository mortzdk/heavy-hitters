#include <stdint.h>
#include <stdio.h>
#include <criterion/criterion.h>

#include "hash.h"
#include "sketch/sketch.h"
#include "hh/hh_sketch.h"

Test(count_min_sketch, expected_d, .disabled=0) {
	double phi     = 0.05;
	double epsilon = 0.01;
	double delta   = 0.2;
	uint32_t m     = pow(2, 9);
	uint32_t b     = 2;

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

	hh_sketch_t *hh = hh_sketch_create(&countMin, &hash31, phi, b, epsilon, 
			delta, m);

	for (int i = 0; i < 10; i++) {
		hh_sketch_update(hh, A[i][0], A[i][1]);
	}

	heavy_hitters_t *result = hh_sketch_query(hh);

	cr_expect_eq(result->count, 4, "Heavy hitters (%d) should be 4", result->count);

	for (uint32_t i = 0; i < result->count; i++) {
		printf("Element %"PRIu32" is a heavy hitter\n", result->hitters[i]);
	}

	hh_sketch_destroy(hh);
}

/*
Test(count_min_sketch, expected_w_16, .disabled=0) {
	sketch_t *s     = sketch_create(&countMin, 2, 0.25, 0.2, &hash31);
	count_min_t *cm = s->sketch;

	cr_assert_eq(cm->w, 8, "Wrong w value, expected %d got %"PRIu32, 8, cm->w);

	sketch_destroy(s);
}

Test(count_min_sketch, update_point, .disabled=0) {
	sketch_t *s = sketch_create(&countMin, 2, 0.3, 0.2, &hash31);

	sketch_update(s, 9, 42);
	uint32_t estimate = sketch_point(s, 9);

	cr_assert_eq(estimate, 42, "Estimate (%d) should be 42", estimate);

	sketch_destroy(s);
}

Test(count_min_sketch, update_point_2, .disabled=0) {
	uint32_t estimate;
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

	sketch_t *s0 = sketch_create(&countMin, 2, 0.25, 0.2, &hash31);
	sketch_t *s1 = sketch_create(&countMin, 2, 0.25, 0.2, &hash31p2);
	sketch_t *s2 = sketch_create(&countMin, 2, 0.25, 0.2, &multiplyShift);

	for (int i = 0; i < 10; i++) {
		sketch_update(s0, A[i][0], A[i][1]);
		sketch_update(s1, A[i][0], A[i][1]);
		sketch_update(s2, A[i][0], A[i][1]);
	}

	for (int i = 0; i < 10; i++) {
		estimate = sketch_point(s0, A[i][0]);
		cr_assert_geq(estimate, A[i][1], "Estimate (%d) should be %d", estimate, A[i][1]);

		estimate = sketch_point(s1, A[i][0]);
		cr_assert_geq(estimate, A[i][1], "Estimate (%d) should be %d", estimate, A[i][1]);
		estimate = sketch_point(s2, A[i][0]);
		cr_assert_geq(estimate, A[i][1], "Estimate (%d) should be %d", estimate, A[i][1]);
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

	sketch_t *s = sketch_create(&countMin, 2, 0.3, 0.2, &hash31);

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

	sketch_t *s = sketch_create(&countMin, 2, 0.3, 0.2, &hash31);

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
	sketch_t *s     = sketch_create(&countMin, 2, 0.5, 0.2, &hash31);
	count_min_t *cm = s->sketch;

	cr_assert_not_null(cm, "Expedted an allocated structure");
	cr_assert_not_null(cm->a, "Expedted an allocated structure");
	cr_assert_not_null(cm->b, "Expedted an allocated structure");
	cr_assert_not_null(cm->table, "Expedted an allocated structure");

	sketch_destroy(s);
}

Test(count_min_sketch, should_free_object, .disabled=1) {
	sketch_t *s = sketch_create(&countMin, 2, 0.5, 0.2, &hash31);
	sketch_destroy(s);

	cr_assert_null(s, "Expedted an empty structure");
}
*/
