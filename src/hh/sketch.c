#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "util/xutil.h"
#include "util/fifo.h"
#include "hh/hh.h"
#include "hh/sketch.h"
#include "sketch/sketch.h"

hh_sketch_t *hh_sketch_create(heavy_hitter_params_t *restrict p) {
	int8_t i;
	uint8_t np2_base;
	uint32_t w, d, top_tree_size;
	hh_sketch_params_t *restrict params = (hh_sketch_params_t *)p->params;
	const uint32_t m           = params->m;
	const uint8_t logm         = xceil_log2(m);
	const double phi           = params->phi;
	const double epsilon       = params->epsilon;
	const uint32_t twophi      = ceil(2./phi);
	double delta               = (double)((params->delta*phi)/(2.*logm));
	const uint32_t b           = params->b;
	const uint32_t result_size = sizeof(uint32_t) * twophi;
	hh_sketch_t *restrict hh   = xmalloc( sizeof(hh_sketch_t) );
	sketch_t    *restrict s    = sketch_create(params->f, p->hash, b, epsilon, 
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
	hh->fifo           = fifo_create(twophi);
	hh->result.size    = twophi;
	hh->result.hitters = xmalloc( result_size );
	memset(hh->result.hitters, '\0', result_size);

	// Find the base (log2) of the next power of two of w*d
	np2_base        = MultiplyDeBruijnBitPosition2[
		(uint32_t)(next_pow_2(w*d) * 0x077CB531U) >> 27
	]; //+ 1; We only do exact when it is below size of sketch

	if (np2_base > logm) {
		np2_base = logm;
	}

	top_tree_size = sizeof(uint64_t) * ((1 << (np2_base+1))-2);
	hh->top       = xmalloc( top_tree_size );
	memset(hh->top, '\0', top_tree_size );
	hh->top_cnt   = np2_base;

	if ( np2_base < logm ) {
		delta    = (double)((params->delta*phi)/(2.*(logm-np2_base)));
		hh->tree = xmalloc( sizeof(sketch_t *) * (logm-np2_base) );
		for (i = 0; i < (logm-1)-np2_base; i++) {
			hh->tree[i] = sketch_create(params->f, p->hash, b, epsilon, delta);
		}
		hh->tree[i] = s;
	} else {
		hh->tree = NULL;
		sketch_destroy(s);
	}

	#ifdef SPACE
	uint64_t space = top_tree_size + result_size + 
		sizeof(sketch_t *) * (logm-np2_base) + sizeof(hh_sketch_t);
	fprintf(stderr, "Space usage excluding sketches: %"PRIu64" bytes\n\n", space);
	#endif

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

	if (hh->fifo != NULL) {
		fifo_destroy(hh->fifo);
		hh->fifo = NULL;
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
	sketch_t **restrict tree = hh->tree;
	uint64_t  *restrict top  = hh->top;
	const uint8_t top_cnt    = hh->top_cnt; 
	const uint8_t logm       = hh->logm;

	x     = 0;
	left  = 0;
	right = hh->params->m-1;

	// Update exact counts as long as |x| <= next_pow_2(wd)
	for (i = 0; i < top_cnt; i++) {
		mid = left + ( (right - left)/2 );
		x   = 2*x;
		if (mid < idx) {
			x++;
			left = mid+1;
		} else {
			right = mid;
		}
		top[x+(1 << (i+1))-2] += c;
	}

	// Use sketches to estimate count instead
	for (i = 0; i < logm-top_cnt; i++) {
		mid = left + ( (right - left)/2 );
		x  *= 2;
		if (mid < idx) {
			x++;
			left = mid+1;
		} else {
			right = mid;
		}
		sketch_update(tree[i], x, c);
	}

	hh->norm += c;
}
	
//Query
static inline void hh_sketch_resize_result(heavy_hitter_t *res) {
	if ( unlikely(res->count >= res->size) ) { 
		res->hitters = xrealloc(res->hitters, res->size*2*sizeof(uint32_t));
		memset(res->hitters+res->size, '\0', res->size*sizeof(uint32_t));
		res->size += res->size;
	}
}

static void hh_sketch_query_bottom_recursive(hh_sketch_t *restrict hh, 
		const uint8_t layer, uint32_t x, const double th) {
	uint8_t i;
	sketch_t **restrict tree = hh->tree;
	const uint8_t top_cnt          = hh->top_cnt; 
	const uint8_t logm             = hh->logm;

	x *= 2;

	for (i = 0; i < 2; i++) {
		x += i;	
		if ( sketch_point(tree[layer], x) >= th ) {
			if ( unlikely( layer+top_cnt == logm-1 ) ) {
				hh->result.hitters[hh->result.count] = x;

				assert( x < hh->params->m );

				hh->result.count++;

				hh_sketch_resize_result(&hh->result);
			} else {
				hh_sketch_query_bottom_recursive(hh, layer+1, x, th);
			}
		}
	}
}

static void hh_sketch_query_top_recursive(hh_sketch_t *restrict hh, 
		const uint8_t layer, uint32_t x, const double th) {
	uint8_t i;
	uint64_t *restrict top = hh->top;
	const uint8_t top_cnt        = hh->top_cnt; 
	const uint8_t logm           = hh->logm;

	x *= 2;

	for (i = 0; i < 2; i++) {
		x += i;	

		if ( top[x+(1 << (layer+1))-2] >= th ) {
			if ( unlikely(layer == logm-1) ) {
				hh->result.hitters[hh->result.count] = x;

				assert( x < hh->params->m );

				hh->result.count++;

				hh_sketch_resize_result(&hh->result);
			} else if ( unlikely(layer == top_cnt-1) ) {
				hh_sketch_query_bottom_recursive(hh, 0, x, th);
			} else {
				hh_sketch_query_top_recursive(hh, layer+1, x, th);
			}
		}
	}
}


heavy_hitter_t *hh_sketch_query(hh_sketch_t *restrict hh) {
	uint32_t x, idx;
	uint8_t i, layer;
	elm_t *res;
	const uint8_t top_cnt    = hh->top_cnt; 
	const uint8_t logm       = hh->logm;
	const double threshold   = hh->params->phi*hh->norm;
	sketch_t **restrict tree = hh->tree;
	uint64_t  *restrict top  = hh->top;
	fifo_t    *restrict fifo = hh->fifo;

	hh->result.count         = 0;

	memset(hh->result.hitters, '\0', hh->result.size); 

	fifo_push_back(fifo, 0, 0);

	while ( !fifo_empty(fifo) ) {
		res   = fifo_pop_front(fifo);
		idx   = 2*res->elm;
		layer = res->layer;

		for (i = 0; i < 2; i++) { // branch=2
			x = idx+i;

			if ( layer < top_cnt  ) {
				if ( top[x+(1 << (layer+1))-2] >= threshold ) {
					if ( unlikely(layer == logm-1) ) {
						hh->result.hitters[hh->result.count] = x;

						assert( x < hh->params->m );

						hh->result.count++;

						hh_sketch_resize_result(&hh->result);
					} else {
						fifo_push_back(fifo, x, layer+1);
					}
				}
			} else {
				if ( sketch_point(tree[layer-top_cnt], x) >= threshold ) {
					if ( unlikely( layer == logm-1 ) ) {
						hh->result.hitters[hh->result.count] = x;

						assert( x < hh->params->m );

						hh->result.count++;

						hh_sketch_resize_result(&hh->result);
					} else {
						fifo_push_back(fifo, x, layer+1);
					}
				} 
			}
		}
	}

	return &hh->result;
}

// Query
heavy_hitter_t *hh_sketch_query_recursive(hh_sketch_t *restrict hh) {
	const double threshold = hh->params->phi*hh->norm;

	hh->result.count = 0;
	memset(hh->result.hitters, '\0', hh->result.size);

	hh_sketch_query_top_recursive(hh, 0, 0, threshold);

	return &hh->result;
}
