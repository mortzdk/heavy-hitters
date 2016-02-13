#include <stdint.h>
#include <string.h>
#include <math.h>

#include "xutil.h"
#include "hash.h"

#include "hh/hh.h"
#include "hh/const_sketch.h"
#include "sketch/sketch.h"

// Initialization
hh_const_sketch_t *hh_const_sketch_create(sketch_func_t *f, hash_t *hash, 
		heavy_hitter_params_t *params) {
	double   phi          = params->phi;
	double   epsilon      = params->epsilon;
	double   delta        = params->delta;
	uint32_t m            = params->m;
	uint32_t b            = params->b;
	hh_const_sketch_t *hh = xmalloc( sizeof(hh_const_sketch_t) );
	const double error    = 0.25;
	uint8_t logm          = xceil_log2(m);
	sketch_t *s           = sketch_create(f, hash, b, epsilon, 
			((delta*(phi+epsilon))/logm));
	uint32_t w            = ceil(sketch_width(s->sketch)/error);
	uint32_t np2_base     = MultiplyDeBruijnBitPosition2[
		(uint32_t)(next_pow_2(w) * 0x077CB531U) >> 27
	] + 1;

	hh->params         = params;
	hh->logm           = logm;
	hh->norm           = 0;
	hh->result.count   = 0; 
	hh->w              = w;
	hh->hash           = hash;
	hh->result.hitters = xmalloc( sizeof(uint32_t) * (2/phi) );
	memset(hh->result.hitters, '\0', sizeof(uint32_t) * (2/phi));
	hh->sketch         = s;
	hh->exact_cnt      = np2_base;
	hh->tree           = xmalloc( sizeof(uint64_t) * 
			((1 << (np2_base+1))+((w/error)*(logm-np2_base))) );

	//TODO allocate a and b for all hash functions

	return hh;
}

// Destruction
void hh_const_sketch_destroy(hh_const_sketch_t *hh) {
	if (hh == NULL) {
		return;
	}

	sketch_destroy(hh->sketch);

	if (hh->tree != NULL) {
		free(hh->tree);
		hh->tree = NULL;
	}

	if (hh->result.hitters != NULL) {
		free(hh->result.hitters);
		hh->result.hitters = NULL;
	}

	free(hh);
	hh = NULL;
}

// Update
void hh_const_sketch_update(hh_const_sketch_t *hh, uint32_t idx, int32_t c) {
	uint8_t i;
	uint32_t left, right, mid, x;

	x     = 0;
	left  = 0;
//	right = next_pow_2(hh->m)-1;
	right = hh->params->m-1;
	mid   = right/2;

	if (mid < idx) {
		x++;
		hh->tree[x] += c;
		left = mid+1;
	} else {
		hh->tree[x] += c;
		right = mid;
	}

	// Update exact counts as long as |x| <= next_pow_2(wd)
	for (i = 1; i < hh->exact_cnt; i++) {
		mid = left + ( (right - left)/2 );
		x   = 2*x;
		if (mid < idx) {
			x++;
			hh->tree[x+(1 << (i+1))-2] += c;
			left = mid+1;
		} else {
			hh->tree[x+(1 << (i+1))-2] += c;
			right = mid;
		}
	}

	// Use sketches to estimate count instead
	// TODO update correct
	for (i = 0; i < hh->logm-hh->exact_cnt; i++) {
		mid = left + ( (right - left)/2 );
		x  *= 2;
		if (mid < idx) {
			x++;
//			sketch_update(hh->tree[i], x, c);
			left = mid+1;
		} else {
//			sketch_update(hh->tree[i], x, c);
			right = mid;
		}
	}

	hh->norm += c;
}

// Query
heavy_hitter_t *hh_const_sketch_query(hh_const_sketch_t *hh) {
	return &hh->result;
}
