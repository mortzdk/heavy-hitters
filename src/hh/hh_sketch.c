#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "xutil.h"
#include "sketch/sketch.h"
#include "hh/hh_sketch.h"

static inline uint32_t sketch_depth(void *sketch) {
	return ((sketch_size_t *)sketch)->d;
}

static inline uint32_t sketch_width(void *sketch) {
	return ((sketch_size_t *)sketch)->w;
}

hh_sketch_t *hh_sketch_create(sketch_func_t *f, hash_t *hash, double phi,
		short b, double epsilon, double delta, uint32_t m){
	int32_t i;
	short np2_base;
	uint32_t w, d;
	sketch_t *s;
	hh_sketch_t *hh = xmalloc( sizeof(hh_sketch_t) );
	short logm      = ceil(log2(m));

	assert(phi > epsilon);
	
//	epsilon         = epsilon/(2*logm);
	delta           = (delta*phi)/(2*logm);
	s               = sketch_create(f, hash, b, epsilon, delta);

	assert(phi > epsilon);

	// TODO: Maybe just calculate w and d again
	// This only works since the sketch_size_t appears first in the *_sketch_t structures!
	w               = sketch_width(s->sketch);
	d               = sketch_depth(s->sketch);

	hh->m              = m;
	hh->logm           = logm;
	hh->phi            = phi;
	hh->epsilon        = epsilon;
	hh->norm           = 0;
	hh->result.count   = 0; 
	hh->result.hitters = xmalloc( sizeof(uint32_t) * (2/phi) );
	memset(hh->result.hitters, '\0', sizeof(uint32_t) * (2/phi));

	// Find the base (log2) of the next power of two of w*d
	np2_base        = MultiplyDeBruijnBitPosition2[
		(uint32_t)(next_pow_2(w*d) * 0x077CB531U) >> 27
	] + 1;

	if (np2_base > logm) {
		np2_base = logm;
	}

	// TODO: Overestimated space usage of top
	hh->top         = xmalloc( sizeof(uint32_t) * (1 << (np2_base+1)) ); // pow(2, np2_base+1)
	hh->top_cnt     = np2_base;

	memset(hh->top, '\0', sizeof(uint32_t) * (1 << (np2_base+1)));

	if ( np2_base < logm ) {
		hh->tree = xmalloc( sizeof(sketch_t *) * (logm-np2_base) );
		for (i = 0; i < (logm-1)-np2_base; i++) {
			hh->tree[i] = sketch_create(f, hash, b, epsilon, delta);
		}
		hh->tree[i] = s;
	} else {
		hh->tree = NULL;
		sketch_destroy(s);
	}

	return hh;
}

// Destuction
void hh_sketch_destroy(hh_sketch_t *hh) {
	short i;

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
void hh_sketch_update(hh_sketch_t *hh, uint32_t idx, int32_t c) {
	short i;
	uint32_t left, right, mid, x;

	x     = 0;
	left  = 0;
//	right = next_pow_2(hh->m)-1;
	right = hh->m-1;
	mid   = right/2;

	if (mid < idx) {
		x++;
		hh->top[x] += c;
		left = mid+1;
	} else {
		hh->top[x] += c;
		right = mid;
	}

	// Update exact counts as long as |x| <= next_pow_2(wd)
	for (i = 1; i < hh->top_cnt; i++) {
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
	
static void hh_sketch_query_bottom_recursive(hh_sketch_t *hh, short layer, 
		uint32_t x, double th) {
	short i;
	uint32_t point;
	x *= 2;

	for (i = 0; i < 2; i++) {
		x += i;	
		if ( (point = sketch_point(hh->tree[layer], x)) >= th ) {
			if ( unlikely( layer+hh->top_cnt == hh->logm-1 ) ) {
				hh->result.hitters[hh->result.count] = x;

				assert( x < hh->m );

				hh->result.count++;
				return;
			}
			hh_sketch_query_bottom_recursive(hh, layer+1, x, th);
		}
	}
}

static void hh_sketch_query_top_recursive(hh_sketch_t *hh, short layer, 
		uint32_t x, double th) {
	short i;
	x *= 2;

	for (i = 0; i < 2; i++) {
		x += i;	

		if ( hh->top[x+(1 << (layer+1))-2] >= th ) {
			if ( unlikely(layer == hh->logm-1) ) {
				hh->result.hitters[hh->result.count] = x;

				assert( x < hh->m );

				hh->result.count++;
			} else if ( unlikely(layer == hh->top_cnt-1) ) {
				hh_sketch_query_bottom_recursive(hh, 0, x, th+(hh->epsilon*hh->norm));
			} else {
				hh_sketch_query_top_recursive(hh, layer+1, x, th);
			}
		}
	}
}

// Query
heavy_hitters_t *hh_sketch_query(hh_sketch_t *hh) {
	double thresshold = hh->phi*hh->norm;

	if (hh->top[0] >= thresshold) {
		hh_sketch_query_top_recursive(hh, 1, 0, thresshold);
	}

	if (hh->top[1] >= thresshold) {
		hh_sketch_query_top_recursive(hh, 1, 1, thresshold);
	}

	return &hh->result;
}
