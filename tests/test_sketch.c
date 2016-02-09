#include <criterion/criterion.h>

#include "sketch.h"

Test(sketch, expected_d, .disabled=0) {
	sketch_t *s = sketch_create(2, 0.25, 0.2);

	cr_assert_eq(s->d, 3, "Wrong d value");

	sketch_destroy(s);
}

Test(sketch, expected_w_16, .disabled=0) {
	sketch_t *s = sketch_create(2, 0.25, 0.2);

	cr_assert_eq(s->w, 16, "Wrong w value");

	sketch_destroy(s);
}

Test(sketch, update_point, .disabled=0) {
	sketch_t *s = sketch_create(2, 0.3, 0.2);

	sketch_update(s, 9, 42);
	uint32_t estimate = sketch_point(s, 9);

	cr_assert_eq(estimate, 42, "Estimate (%d) should be 42", estimate);

	sketch_destroy(s);
}

Test(sketch, update_point_2, .disabled=0) {
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

	sketch_t *s = sketch_create(2, 0.25, 0.2);

	for (int i = 0; i < 10; i++) {
		sketch_update(s, A[i][0], A[i][1]);
	}

	for (int i = 0; i < 10; i++) {
		estimate = sketch_point(s, A[i][0]);
		cr_assert_geq(estimate, A[i][1], "Estimate (%d) should be %d", estimate, A[i][1]);
	}

	sketch_destroy(s);
}

Test(sketch, update_point_3, .disabled=0) {
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

	sketch_t *s = sketch_create(2, 0.3, 0.2);

	for (int i = 0; i < 10; i++) {
		sketch_update(s, A[i][0], A[i][1]);
	}

	uint32_t sum = 0;

	for (int i = 0; i < 10; i++) {
		sum += A[i][1];
	}

	estimate = sketch_range_naive(s, 42, 42);
	cr_assert_eq(estimate, sum, "Estimate (%d) should be %d", estimate, sum);

	estimate = sketch_range_naive(s, 0, 100);
	cr_assert_eq(estimate, sum, "Estimate (%d) should be %d", estimate, sum);

	sketch_destroy(s);
}

Test(sketch, update_range_naive_1, .disabled=0) {
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

	sketch_t *s = sketch_create(2, 0.3, 0.2);

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

Test(sketch, should_allocate_internals, .disabled=0) {
	sketch_t *s = sketch_create(2, 0.5, 0.2);

	cr_assert_not_null(s, "Expedted an allocated structure");
	cr_assert_not_null(s->a, "Expedted an allocated structure");
	cr_assert_not_null(s->b, "Expedted an allocated structure");
	cr_assert_not_null(s->table, "Expedted an allocated structure");

	sketch_destroy(s);
}

Test(sketch, should_free_object, .disabled=1) {
	sketch_t *s = sketch_create(2, 0.5, 0.2);
	sketch_destroy(s);

	cr_assert_null(s, "Expedted an empty structure");
}
