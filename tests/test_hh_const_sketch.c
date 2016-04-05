#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <criterion/criterion.h>

#include "util/hash.h"
#include "util/alias.h"
#include "util/xutil.h"

#include "hh/hh.h"
#include "hh/const_sketch.h"
#include "sketch/sketch.h"

Test(hh_const_sketch, hh_top_only, .disabled=0) {
	uint32_t A[10][2] = {
		{1, 3543},
		{2, 7932},
		{3, 8234},
		{4, 48},
		{5, 58},
		{6, 238},
		{7, 732},
		{8, 10038},
		{9, 78},
		{327, 78923}
	};

	uint32_t H[4] = {  // Expected heavy hitters
		2, 3, 8, 327
	};

	hh_const_sketch_params_t params = {
		.b       = 2,
		.epsilon = 0.01,
		.delta   = 0.2,
		.m       = pow(2, 9),
		.phi     = 0.05,
		.f       = &countMin,
	};
	heavy_hitter_params_t p = {
		.hash   = &hash31,
		.params = &params,
		.f      = &hh_const_sketch,
	};
	hh_t *hh = heavy_hitter_create(&p);

	for (int i = 0; i < 10; i++) {
		heavy_hitter_update(hh, A[i][0], A[i][1]);
	}

	heavy_hitter_t *result = heavy_hitter_query(hh);

	cr_assert_eq(result->count, 4, "Heavy hitters (%d) should be 4", result->count);

	for (uint32_t i = 0; i < result->count; i++) {
		cr_expect_eq(
				H[i], 
				result->hitters[i], 
				"Expected %"PRIu32" to be next heavy hitter got: %"PRIu32, 
				H[i], 
				result->hitters[i]
		);
	}

	heavy_hitter_destroy(hh);
}

Test(hh_const_sketch, hh_top_and_bottom, .disabled=0) {
	double hh_mass   = 0.70;
	uint32_t m       = pow(2, 20);

	hh_const_sketch_params_t params = {
		.b       = 2,
		.epsilon = (double)1/64,
		.delta   = 0.2,
		.m       = m,
		.phi     = 0.05,
		.f       = &countMin,
	};
	heavy_hitter_params_t p = {
		.hash   = &multiplyShift,
		.params = &params,
		.f      = &hh_const_sketch,
	};

	hh_t *hh = heavy_hitter_create(&p);
	double *x       = xmalloc( m*sizeof(double) );

	for (uint32_t i = 0; i < m; i++) {
		x[i] = (1-hh_mass)/(m-7);
	}

	/**
	 * 7 heavy hitters
	 */
	x[134]     = 0.10;
	x[2345]    = 0.10;
	x[374298]  = 0.10;
	x[374299]  = 0.10;
	x[1000000] = 0.10;
	x[38474]   = 0.10;
	x[3]       = 0.10;

	alias_t * a = alias_preprocess(m, x);

	uint32_t idx;
	for (uint32_t i = 0; i < pow(2, 25); i++) {
		idx = alias_draw(a);
		heavy_hitter_update(hh, idx, 1);
	}

	heavy_hitter_t *result = heavy_hitter_query(hh);

	cr_assert_eq(result->count, 7, "Heavy hitters (%d) should be 7", result->count);

	uint32_t H[7] = {  // Expected heavy hitters
		3, 134, 2345, 38474, 374298, 374299, 1000000
	};
	for (uint32_t i = 0; i < result->count; i++) {
		cr_expect_eq(
				H[i], 
				result->hitters[i], 
				"Expected %"PRIu32" to be next heavy hitter got: %"PRIu32, 
				H[i], 
				result->hitters[i]
		);
	}

	heavy_hitter_destroy(hh);
}

Test(hh_const_sketch, hh_top_and_bottom_close_non_hh, .disabled=0) {
	double hh_mass   = 0.7 + 0.04955 + 0.04812 + 0.05023;
	uint32_t m       = pow(2, 20);

	hh_const_sketch_params_t params = {
		.b       = 2,
		.epsilon = (double)1/128,
		.delta   = 0.2,
		.m       = m,
		.phi     = 0.05,
		.f       = &countMin,
	};
	heavy_hitter_params_t p = {
		.hash   = &multiplyShift,
		.params = &params,
		.f      = &hh_const_sketch,
	};

	hh_t *hh = heavy_hitter_create(&p);
	double *x       = xmalloc( m*sizeof(double) );

	for (uint32_t i = 0; i < m; i++) {
		x[i] = (1-hh_mass)/(m-7);
	}

	/**
	 * 7 heavy hitters
	 */
	x[134]     = 0.10;
	x[2345]    = 0.10;
	x[374298]  = 0.10;
	x[374299]  = 0.10;
	x[1000000] = 0.10;
	x[38474]   = 0.10;
	x[3]       = 0.10;
	x[737449]  = 0.05023;

	/**
	 * High but not heavy hitters
	 */
	x[5983]    = 0.04955;
	x[389449]  = 0.04812;

	alias_t * a = alias_preprocess(m, x);

	uint32_t idx;
	for (uint32_t i = 0; i < pow(2, 25); i++) {
		idx = alias_draw(a);
		heavy_hitter_update(hh, idx, 1);
	}

	heavy_hitter_t *result = heavy_hitter_query(hh);

	cr_expect_eq(result->count, 8, "Heavy hitters (%d) should be 8", result->count);

	uint32_t H[8] = {  // Expected heavy hitters
		3, 134, 2345, 38474, 374298, 374299, 737449, 1000000
	};
	for (uint32_t i = 0; i < result->count; i++) {
		cr_expect_eq(
				H[i], 
				result->hitters[i], 
				"Expected %"PRIu32" to be next heavy hitter got: %"PRIu32, 
				H[i], 
				result->hitters[i]
		);
	}

	heavy_hitter_destroy(hh);
}
