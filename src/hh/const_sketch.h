#ifndef H_hh_const_sketch
#define H_hh_const_sketch

// Standard libraries
#include <stdint.h>

// User defined libraries
#include "hash.h"
#include "hh/hh.h"
#include "sketch/sketch.h"

// Structures
typedef struct {
	double          phi;
	double          epsilon;
	double          delta;
	uint32_t        m;
	uint32_t        b;
	sketch_func_t *f;
} hh_const_sketch_params_t;

typedef struct {
	uint64_t                 *restrict tree;
	sketch_t                 *restrict sketch;
	hash_t                   *restrict hash;
	uint8_t                   exact_cnt;
	uint32_t                  w;
	short                     logm;
	uint64_t                  norm;
	hh_const_sketch_params_t *restrict params;
	heavy_hitter_t            result;
} hh_const_sketch_t; 

// Initialization
hh_const_sketch_t *hh_const_sketch_create(heavy_hitter_params_t *restrict p);

// Destruction
void hh_const_sketch_destroy(hh_const_sketch_t *restrict hh);

// Update
void hh_const_sketch_update(hh_const_sketch_t *restrict hh, const uint32_t idx, 
		const int64_t c);

// Query
heavy_hitter_t *hh_const_sketch_query(hh_const_sketch_t *restrict hh);

#endif
