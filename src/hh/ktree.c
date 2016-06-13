#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "util/xutil.h"
#include "util/fifo.h"
#include "hh/hh.h"
#include "hh/ktree.h"
#include "sketch/sketch.h"

hh_ktree_t *hh_ktree_create(heavy_hitter_params_t *restrict p) {
	int8_t i;
	uint32_t w, d, wd, top_tree_size, size;
	hh_ktree_params_t *restrict params = (hh_ktree_params_t *)p->params;
	uint8_t top_cnt            = 1;
	const uint8_t gran         = params->gran;
	const uint32_t k           = (1 << gran);
	const uint32_t m           = params->m;
	const uint8_t logm         = floor(log((uint64_t)m)/log(k)+1);
	const double phi           = params->phi;
	const double epsilon       = params->epsilon;
	const uint32_t queries     = ceil((double)k/phi);
	double delta               = (double)((params->delta*phi)/(k*logm));
	const uint32_t b           = params->b;
	const uint32_t result_cnt  = ceil(1./phi);
	const uint32_t result_size = sizeof(uint32_t) * result_cnt;
	hh_ktree_t *restrict hh    = xmalloc( sizeof(hh_ktree_t) );
	sketch_t   *restrict s     = sketch_create(params->f, p->hash, b, epsilon, 
			delta);

	assert(phi > epsilon);
	
	// This only works since the sketch_size_t appears first in the *_sketch_t 
	// structures!
	w                  = sketch_width(s->sketch);
	d                  = sketch_depth(s->sketch);
	wd                 = w*d;

	hh->logm           = logm;
	hh->params         = params;
	hh->gran           = gran;
	hh->k              = k;
	hh->norm           = 0;
	hh->result.count   = 0; 
	hh->fifo           = fifo_create(queries);
	hh->result.size    = result_cnt;
	hh->result.hitters = xmalloc( result_size );
	memset(hh->result.hitters, '\0', result_size);

	// Find the base of the next power of k of w*d
	for (i = 1; ((uint64_t)k << i*gran) <= wd; i++) {
		top_cnt++;
	}

	if ( unlikely(top_cnt >= logm) ) {
		top_cnt = logm;
		size    = (uint32_t)(floor((pow(k,top_cnt)-1)/(k-1))-1)+m;
	} else {
		size    = (uint32_t)(floor((pow(k,top_cnt+1)-1)/(k-1))-1);
	}

	top_tree_size = sizeof(uint64_t) * size;
	hh->top       = xmalloc( top_tree_size );
	memset(hh->top, '\0', top_tree_size);
	hh->top_cnt   = top_cnt;


	if ( top_cnt < logm ) {
		hh->tree = xmalloc( sizeof(sketch_t *) * (logm-top_cnt) );
		for (i = logm-top_cnt-1; i > 0; i--) {
			hh->tree[i] = sketch_create(params->f, p->hash, b, epsilon, delta);
		}
		hh->tree[i] = s;
	} else {
		hh->tree = NULL;
		sketch_destroy(s);
	}

	#ifdef SPACE
	uint64_t space = top_tree_size + result_size + 
		sizeof(sketch_t *) * (logm-top_cnt) + sizeof(hh_ktree_t);
	fprintf(stderr, "Space usage excluding sketches: %"PRIu64" bytes\n\n", space);
	#endif

	return hh;
}

// Destuction
void hh_ktree_destroy(hh_ktree_t *restrict hh) {
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
void hh_ktree_update(hh_ktree_t *restrict hh, const uint32_t idx, 
		const int64_t c) {
	int8_t i;
	uint32_t x;
	sketch_t **restrict tree = hh->tree;
	uint64_t  *restrict top  = hh->top;
	const uint8_t top_cnt    = hh->top_cnt; 
	const uint8_t logm       = hh->logm;
	const uint8_t gran       = hh->gran; 
	const uint32_t k         = hh->k;

	x = idx;

	// Use sketches to estimate count instead
	for (i = logm-top_cnt-1; i > -1; i--) {
		sketch_update(tree[i], x, c);
		x >>= gran;
	}

	for (i = top_cnt-1; i > -1; i--) {
		top[x + (uint32_t)((k << (i*gran))-1)/(k-1) - 1] += c;
		x >>= gran;
	}

	hh->norm += c;
}
	
//Query
static inline void hh_ktree_resize_result(heavy_hitter_t *res) {
	if ( unlikely(res->count >= res->size) ) { 
		res->hitters = xrealloc(res->hitters, res->size*2*sizeof(uint32_t));
		memset(res->hitters+res->size, '\0', res->size*sizeof(uint32_t));
		res->size += res->size;
	}
}

heavy_hitter_t *hh_ktree_query(hh_ktree_t *restrict hh) {
	uint32_t x, idx, tmp, i;
	uint8_t layer;
	elm_t *res;
	const uint8_t top_cnt    = hh->top_cnt; 
	const uint8_t logm       = hh->logm;
	const uint8_t gran       = hh->gran;
	const uint32_t k         = hh->k;
	const double threshold   = hh->params->phi*hh->norm;
	sketch_t **restrict tree = hh->tree;
	uint64_t  *restrict top  = hh->top;
	fifo_t    *restrict fifo = hh->fifo;

	hh->result.count         = 0;

	memset(hh->result.hitters, '\0', hh->result.size); 

	fifo_push_back(fifo, 0, 0);

	while ( !fifo_empty(fifo) ) {
		res   = fifo_pop_front(fifo);
		idx   = res->elm;
		layer = res->layer;

		for (i = 0; i < k; i++) { // branches
			x = idx+i;

			if ( layer < top_cnt  ) {
				tmp = (uint32_t)((k << (layer*gran))-1)/(k-1) - 1;
				if ( top[x+tmp] >= threshold ) {
					if ( unlikely(layer == logm-1) ) {
						hh->result.hitters[hh->result.count] = x;

						assert( x <= hh->params->m );

						hh->result.count++;

						hh_ktree_resize_result(&hh->result);
					} else {
						fifo_push_back(fifo, x<<gran, layer+1);
					}
				}
			} else {
				if ( sketch_point(tree[layer-top_cnt], x) >= threshold ) {
					if ( unlikely( layer == logm-1 ) ) {
						hh->result.hitters[hh->result.count] = x;

						assert( x <= hh->params->m );

						hh->result.count++;

						hh_ktree_resize_result(&hh->result);
					} else {
						fifo_push_back(fifo, x<<gran, layer+1);
					}
				} 
			}
		}
	}

	return &hh->result;
}
