#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#include "xutil.h"
#include "hash.h"

#include "hh/hh.h"
#include "hh/const_sketch.h"
#include "sketch/sketch.h"

// Initialization
hh_const_sketch_t *hh_const_sketch_create(heavy_hitter_params_t *restrict p) {
	uint32_t i, size;
	hh_const_sketch_params_t *restrict params = 
		(hh_const_sketch_params_t *)p->params;
	const double   phi     = params->phi;
	const double   epsilon = params->epsilon;
	const double   delta   = params->delta;
	const uint32_t m       = params->m;
	const uint32_t b       = params->b;
	const double error     = 0.25;
	const uint8_t logm     = xceil_log2(m);
	hh_const_sketch_t *restrict hh  = xmalloc( sizeof(hh_const_sketch_t) );
	sketch_t          *restrict s   = sketch_create(params->f, p->hash, b,
			(epsilon * p->hash->c),
			((delta*(phi+epsilon))/(logm * 2)));
	const uint32_t w      = ceil(sketch_width(s->sketch)/error);
	uint8_t np2_base      = MultiplyDeBruijnBitPosition2[
		(uint32_t)(next_pow_2(w) * 0x077CB531U) >> 27
	] + 1;

	hash_init(&hh->M, w);

	if (np2_base > logm) {
		np2_base = logm;
	}

	size               = ( ((1 << (np2_base+1))-2) + ((1+w)*(logm-np2_base)) );
	hh->params         = params;
	hh->logm           = logm;
	hh->norm           = 0;
	hh->result.count   = 0; 
	hh->w              = w;
	hh->hash           = p->hash;
	hh->result.hitters = xmalloc( sizeof(uint32_t) * (2/phi) );
	memset(hh->result.hitters, '\0', sizeof(uint32_t) * (2/phi));
	hh->sketch         = s;
	hh->exact_cnt      = np2_base;
	hh->tree           = xmalloc( sizeof(uint64_t) * size );
	memset(hh->tree, '\0', sizeof(uint64_t) * size);

	//TODO allocate a and b for all hash functions
	for (i = (1 << (np2_base+1))-2; i < size; i += 1+w) {
		hh->tree[i] |= ((uint64_t) p->hash->agen()) << 32;
		hh->tree[i] |= (uint64_t) p->hash->bgen(hh->M);
	}

	return hh;
}

// Destruction
void hh_const_sketch_destroy(hh_const_sketch_t *restrict hh) {
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
void hh_const_sketch_update(hh_const_sketch_t *restrict hh, const uint32_t idx, 
		const int64_t c) {
	uint8_t i;
	uint32_t left, right, mid, offset, hash, x, a, b;

	x     = 0;
	left  = 0;
	right = hh->params->m-1;

	// Update exact counts as long as |x| <= next_pow_2(wd)
	for (i = 0; i < hh->exact_cnt; i++) {
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

	offset = (1 << (i+1))-2;

	// Use sketches to estimate count instead
	for (i = 0; i < hh->logm-hh->exact_cnt; i++) {
		mid  = left + ( (right - left)/2 );
		x   *= 2;
		a    = (hh->tree[offset] >> 32);
		b    = (uint32_t) hh->tree[offset];

		if (mid < idx) {
			x++;
			hash = hh->hash->hash(hh->w, hh->M, x, a, b);
			hh->tree[offset + 1 + hash] += c; 
			left = mid+1;
		} else {
			hash = hh->hash->hash(hh->w, hh->M, x, a, b);
			hh->tree[offset + 1 + hash] += c; 
			right = mid;
		}

		offset += (hh->w +1);
	}

	sketch_update(hh->sketch, x, c);

	hh->norm += c;
}

// Query
static void hh_sketch_query_bottom_recursive(hh_const_sketch_t *restrict hh, 
		const uint8_t layer, uint32_t x, const double th) {
	uint8_t i;
	uint32_t offset, a, b, hash;
	x *= 2;

	offset = (1 << (hh->exact_cnt +1))-2   + ((hh->w+1) * layer);

	for (i = 0; i < 2; i++) {
		x += i;	
		a    = (hh->tree[offset] >> 32);
		b    = (uint32_t) hh->tree[offset];
		hash = hh->hash->hash(hh->w, hh->M, x, a, b);

		// Plus one to get away from a and b
		if ( hh->tree[offset + 1 + hash] >= th ) {
			if ( unlikely( layer+hh->exact_cnt == hh->logm-1 ) ) {
				if ( sketch_above_thresshold(hh->sketch, x, th) ) {
					hh->result.hitters[hh->result.count] = x;

					assert( x < hh->params->m );

					hh->result.count++;
				}
			} else {
				hh_sketch_query_bottom_recursive(hh, layer+1, x, th);
			}
		}
	}
}

static void hh_sketch_query_top_recursive(hh_const_sketch_t *restrict hh, 
		const uint8_t layer, uint32_t x, const double th) {
	uint8_t i;
	x *= 2;

	for (i = 0; i < 2; i++) {
		x += i;	

		if ( hh->tree[x+(1 << (layer+1))-2] >= th ) {
			if ( unlikely(layer == hh->logm-1) ) {
				hh->result.hitters[hh->result.count] = x;

				assert( x < hh->params->m );

				hh->result.count++;
			} else if ( unlikely(layer == hh->exact_cnt-1) ) {
				hh_sketch_query_bottom_recursive(hh, 0, x, th+(hh->params->epsilon*hh->norm));
			} else {
				hh_sketch_query_top_recursive(hh, layer+1, x, th);
			}
		}
	}
}

heavy_hitter_t *hh_const_sketch_query(hh_const_sketch_t *restrict hh) {
	const double thresshold = hh->params->phi*hh->norm;

	hh_sketch_query_top_recursive(hh, 0, 0, thresshold);

	return &hh->result;
}
