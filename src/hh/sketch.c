#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "xutil.h"
#include "hh/hh.h"
#include "hh/sketch.h"
#include "sketch/sketch.h"

hh_sketch_t *hh_sketch_create(heavy_hitter_params_t *restrict p) {
	int32_t i;
	uint8_t np2_base;
	uint32_t w, d, result_size, top_tree_size;
	hh_sketch_params_t *restrict params = (hh_sketch_params_t *)p->params;
	const uint32_t m         = params->m;
	const uint8_t logm       = xceil_log2(m);
	const double   phi       = params->phi;
	const double   epsilon   = params->epsilon;
	const double   delta     = (params->delta*phi)/(2*logm);
	const uint32_t b         = params->b;
	hh_sketch_t *restrict hh = xmalloc( sizeof(hh_sketch_t) );
	sketch_t    *restrict s  = sketch_create(params->f, p->hash, b, epsilon, 
			delta);

	assert(phi > epsilon);
	
	// This only works since the sketch_size_t appears first in the *_sketch_t 
	// structures!
	w                  = sketch_width(s->sketch);
	d                  = sketch_depth(s->sketch);

	hh->logm           = logm;
	hh->params         = params;
	hh->norm           = 0;
	hh->result.count   = 0; 
	result_size        = sizeof(uint32_t) * (2/phi);
	hh->result.hitters = xmalloc( result_size );
	memset(hh->result.hitters, '\0', result_size);

	// Find the base (log2) of the next power of two of w*d
	np2_base        = MultiplyDeBruijnBitPosition2[
		(uint32_t)(next_pow_2(w*d) * 0x077CB531U) >> 27
	] + 1;

	if (np2_base > logm) {
		np2_base = logm;
	}

	top_tree_size = sizeof(uint32_t) * ((1 << (np2_base+1))-2);
	hh->top       = xmalloc( top_tree_size );
	memset(hh->top, '\0', top_tree_size );
	hh->top_cnt   = np2_base;

	if ( np2_base < logm ) {
		hh->tree = xmalloc( sizeof(sketch_t *) * (logm-np2_base) );
		for (i = 0; i < (logm-1)-np2_base; i++) {
			hh->tree[i] = sketch_create(params->f, p->hash, b, epsilon, delta);
		}
		hh->tree[i] = s;
	} else {
		hh->tree = NULL;
		sketch_destroy(s);
	}

	return hh;
}

// Destuction
void hh_sketch_destroy(hh_sketch_t *restrict hh) {
	uint8_t i;

	if (hh == NULL) {
		return;
	}

	if (hh->top != NULL) {
		free(hh->top);
		hh->top = NULL;
	}

	if (hh->result.hitters != NULL) {
		free(hh->result.hitters);
		hh->result.hitters = NULL;
	}

	for (i = 0; i < hh->logm-hh->top_cnt; i++) {
		sketch_destroy(hh->tree[i]);
	}

	free(hh);
	hh = NULL;
}

// Update
void hh_sketch_update(hh_sketch_t *restrict hh, const uint32_t idx, 
		const int64_t c) {
	uint8_t i;
	uint32_t left, right, mid, x;

	x     = 0;
	left  = 0;
	right = hh->params->m-1;

	// Update exact counts as long as |x| <= next_pow_2(wd)
	for (i = 0; i < hh->top_cnt; i++) {
		mid = left + ( (right - left)/2 );
		x   = 2*x;
		if (mid < idx) {
			x++;
			hh->top[x+(1 << (i+1))-2] += c;
			left = mid+1;
		} else {
			hh->top[x+(1 << (i+1))-2] += c;
			right = mid;
		}
	}

	// Use sketches to estimate count instead
	for (i = 0; i < hh->logm-hh->top_cnt; i++) {
		mid = left + ( (right - left)/2 );
		x  *= 2;
		if (mid < idx) {
			x++;
			sketch_update(hh->tree[i], x, c);
			left = mid+1;
		} else {
			sketch_update(hh->tree[i], x, c);
			right = mid;
		}
	}

	hh->norm += c;
}
	
static void hh_sketch_query_bottom_recursive(hh_sketch_t *restrict hh, 
		const uint8_t layer, uint32_t x, const double th) {
	uint8_t i;
	x *= 2;

	for (i = 0; i < 2; i++) {
		x += i;	
		if ( sketch_point(hh->tree[layer], x) >= th ) {
			if ( unlikely( layer+hh->top_cnt == hh->logm-1 ) ) {
				hh->result.hitters[hh->result.count] = x;

				assert( x < hh->params->m );

				hh->result.count++;
			} else {
				hh_sketch_query_bottom_recursive(hh, layer+1, x, th);
			}
		}
	}
}

static void hh_sketch_query_top_recursive(hh_sketch_t *restrict hh, 
		const uint8_t layer, uint32_t x, const double th) {
	uint8_t i;
	x *= 2;

	for (i = 0; i < 2; i++) {
		x += i;	

		if ( hh->top[x+(1 << (layer+1))-2] >= th ) {
			if ( unlikely(layer == hh->logm-1) ) {
				hh->result.hitters[hh->result.count] = x;

				assert( x < hh->params->m );

				hh->result.count++;
			} else if ( unlikely(layer == hh->top_cnt-1) ) {
				hh_sketch_query_bottom_recursive(hh, 0, x, sketch_thresshold(
							hh->tree[0], hh->norm, hh->params->epsilon, th));
			} else {
				hh_sketch_query_top_recursive(hh, layer+1, x, th);
			}
		}
	}
}

// Query
heavy_hitter_t *hh_sketch_query(hh_sketch_t *restrict hh) {
	const double thresshold = hh->params->phi*hh->norm;

	hh_sketch_query_top_recursive(hh, 0, 0, thresshold);

	return &hh->result;
}
