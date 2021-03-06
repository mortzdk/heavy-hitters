#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "hh/hh.h"
#include "hh/const_sketch.h"
#include "sketch/sketch.h"
#include "util/fifo.h"
#include "util/hash.h"
#include "util/xutil.h"

// Initialization
hh_const_sketch_t *hh_const_sketch_create(heavy_hitter_params_t *restrict p) {
	uint32_t i, size;
	hh_const_sketch_params_t *restrict params = 
		(hh_const_sketch_params_t *)p->params;
	const double   phi         = params->phi;
	const double   epsilon     = params->epsilon;
	const double   delta       = params->delta;
	const uint32_t m           = params->m;
	const uint32_t b           = params->b;
	const double error         = 0.25; // 1./4.
	const uint8_t logm         = log2((uint64_t)m+1);
	const uint32_t result_size = sizeof(uint32_t) * ceil(2./phi);
	const uint32_t w           = ceil(1. / (epsilon * error)); // b/(epsilon*error*b)
	uint8_t np2_base           = MultiplyDeBruijnBitPosition2[
		(uint32_t)(next_pow_2(w) * 0x077CB531U) >> 27
	]; //We only do exact when it is below size of sketch

	hh_const_sketch_t *restrict hh  = xmalloc( sizeof(hh_const_sketch_t) );
	sketch_t          *restrict s   = sketch_create(params->f, p->hash, b,
			epsilon, (double)((pow(delta,2)*phi)/(16.)));


	hash_init(&hh->M, w);

	if (np2_base > logm) {
		np2_base = logm;
	}

	size               = ( ((1 << (np2_base+1))-2) + ((2+w)*(logm-np2_base)) );
	hh->params         = params;
	hh->logm           = logm;
	hh->norm           = 0;
	hh->result.count   = 0; 
	hh->w              = w;
	hh->hash           = p->hash;
	hh->fifo           = fifo_create(ceil(3./phi));
	hh->result.hitters = xmalloc(result_size);
	hh->result.size    = ceil(2./phi);
	memset(hh->result.hitters, '\0', result_size);
	hh->sketch         = s;
	hh->exact_cnt      = np2_base;
	hh->tree           = xmalloc( sizeof(uint64_t) * size );
	memset(hh->tree, '\0', sizeof(uint64_t) * size);

	for (i = (1 << (np2_base+1))-2; i < size; i += 2+w) {
		hh->tree[i]   = (uint64_t) p->hash->agen();
		hh->tree[i+1] = (uint64_t) p->hash->bgen(hh->M);
	}

	#ifdef SPACE
	uint64_t space = size * sizeof(uint64_t) + result_size + 
		sizeof(hh_const_sketch_t);
	fprintf(stderr, "Space usage excluding sketches: %"PRIu64" bytes\n\n", space);
	#endif

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

	if (hh->fifo != NULL) {
		fifo_destroy(hh->fifo);
		hh->fifo = NULL;
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
	int8_t i;
	uint32_t off, offset, h, x, a, b;
	const uint8_t  exact_cnt = hh->exact_cnt;
	const uint8_t  logm      = hh->logm;
	const uint8_t  M         = hh->M;
	const uint32_t w         = hh->w;
	uint64_t *restrict tree  = hh->tree;
	hash hash                = hh->hash->hash;

	x      = idx;
	offset = (2 << exact_cnt)-2;

	sketch_update(hh->sketch, x, c);

	// Use sketches to estimate count instead
	for (i = logm-exact_cnt-1; i > -1; i--) {
		off = offset + (2+w)*i;
		a   = (uint64_t) tree[off];
		b   = (uint64_t) tree[off + 1];
		h   = hash(w, M, x, a, b);
		tree[off + 2 + h] += c; 
		x >>= 1;
	}

	// Update exact counts as long as |x| <= next_pow_2(wd)
	for (i = exact_cnt-1; i > -1; i--) {
		tree[x+(2 << i)-2] += c;
		x >>= 1;
	}

	hh->norm += c;
}

// Query
static inline void hh_const_sketch_resize_result(heavy_hitter_t *res) {
	if ( unlikely(res->count >= res->size) ) { 
		res->hitters = xrealloc(res->hitters, res->size*2*sizeof(uint32_t));
		memset(res->hitters+res->size, '\0', res->size*sizeof(uint32_t));
		res->size += res->size;
	}
}

static void hh_const_sketch_query_bottom_recursive(
		hh_const_sketch_t *restrict hh, const uint8_t layer, uint32_t x, 
		const double th) {
	uint8_t i;
	uint32_t offset, a, b, h;
	uint8_t  exact_cnt      = hh->exact_cnt;
	uint8_t  logm           = hh->logm;
	uint8_t  M              = hh->M;
	uint32_t w              = hh->w;
	uint64_t *restrict tree = hh->tree;
	hash hash               = hh->hash->hash;

	x *= 2;

	offset = (1 << (exact_cnt +1))-2 + ((w+2) * layer);

	for (i = 0; i < 2; i++) {
		x += i;	
		a = (uint64_t) tree[offset];
		b = (uint64_t) tree[offset+1];
		h = hash(w, M, x, a, b);

		// Plus one to get away from a and b
		if ( tree[offset + 2 + h] >= th ) {
			if ( unlikely( layer+exact_cnt == logm-1 ) ) {
				if ( sketch_above_thresshold(hh->sketch, x, th) ) {
					hh->result.hitters[hh->result.count] = x;

					assert( x <= hh->params->m );

					hh->result.count++;

					hh_const_sketch_resize_result(&hh->result);
				}
			} else {
				hh_const_sketch_query_bottom_recursive(hh, layer+1, x, th);
			}
		}
	}
}

static void hh_const_sketch_query_top_recursive(hh_const_sketch_t *restrict hh, 
		const uint8_t layer, uint32_t x, const double th) {
	uint8_t i;
	uint8_t exact_cnt       = hh->exact_cnt;
	uint8_t logm            = hh->logm;
	uint64_t *restrict tree = hh->tree;

	x *= 2;

	for (i = 0; i < 2; i++) {
		x += i;	

		if ( tree[x+(1 << (layer+1))-2] >= th ) {
			if ( unlikely(layer == logm-1) ) {
				hh->result.hitters[hh->result.count] = x;

				assert( x <= hh->params->m );

				hh->result.count++;

				hh_const_sketch_resize_result(&hh->result);
			} else if ( unlikely(layer == exact_cnt-1) ) {
				hh_const_sketch_query_bottom_recursive(hh, 0, x, th);
			} else {
				hh_const_sketch_query_top_recursive(hh, layer+1, x, th);
			}
		}
	}
}


heavy_hitter_t *hh_const_sketch_query(hh_const_sketch_t *restrict hh) {
	uint32_t x, idx, offset, a, b, h;
	uint8_t i, layer;
	elm_t *res;
	uint8_t  M               = hh->M;
	uint32_t w               = hh->w;
	uint8_t  exact_cnt       = hh->exact_cnt;
	const uint8_t logm       = hh->logm;
	const double threshold   = hh->params->phi*hh->norm;
	uint64_t *restrict tree  = hh->tree;
	fifo_t    *restrict fifo = hh->fifo;
	hash hash                = hh->hash->hash;

	hh->result.count         = 0;

	memset(hh->result.hitters, '\0', hh->result.size); 

	fifo_push_back(fifo, 0, 0);

	while ( !fifo_empty(fifo) ) {
		res   = fifo_pop_front(fifo);
		idx   = res->elm;
		layer = res->layer;

		for (i = 0; i < 2; i++) { // branch=2
			x = idx+i;

			if ( layer < exact_cnt ) {
				if ( tree[x+(2 << layer)-2] >= threshold ) {
					if ( unlikely(layer == logm-1) ) {
						hh->result.hitters[hh->result.count] = x;

						assert( x <= hh->params->m );

						hh->result.count++;

						hh_const_sketch_resize_result(&hh->result);
					} else {
						fifo_push_back(fifo, x<<1, layer+1);
					}
				}
			} else {
				offset = (2 << exact_cnt)-2 + ((w+2) * (layer-exact_cnt));
				a = (uint64_t) tree[offset];
				b = (uint64_t) tree[offset+1];
				h = hash(w, M, x, a, b);

				// Plus two to get away from a and b
				if ( tree[offset + 2 + h] >= threshold ) {
					if ( unlikely( layer == logm-1 ) ) {
						if ( sketch_above_thresshold(hh->sketch, x, 
									threshold) ) {
							hh->result.hitters[hh->result.count] = x;

							assert( x <= hh->params->m );

							hh->result.count++;

							hh_const_sketch_resize_result(&hh->result);
						}
					} else {
						fifo_push_back(fifo, x<<1, layer+1);
					}
				}
			}
		}
	}

	return &hh->result;
}

heavy_hitter_t *hh_const_sketch_query_recursive(hh_const_sketch_t *restrict hh) {
	const double thresshold = hh->params->phi*hh->norm;

	hh->result.count = 0;
	memset(hh->result.hitters, '\0', hh->result.size);

	hh_const_sketch_query_top_recursive(hh, 0, 0, thresshold);

	return &hh->result;
}
