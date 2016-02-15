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
	uint64_t              *tree;
	sketch_t              *sketch;
	hash_t                *hash;
	uint8_t                exact_cnt;
	uint32_t               w;
	short                  logm;
	uint64_t               norm;
	heavy_hitter_params_t *params;
	heavy_hitter_t         result;
} hh_const_sketch_t; 

// Initialization
hh_const_sketch_t *hh_const_sketch_create(sketch_func_t *f, hash_t *hash, 
		heavy_hitter_params_t *params);

// Destruction
void hh_const_sketch_destroy(hh_const_sketch_t *hh);

// Update
void hh_const_sketch_update(hh_const_sketch_t *hh, uint32_t idx, int64_t c);

// Query
heavy_hitter_t *hh_const_sketch_query(hh_const_sketch_t *hh);

#endif
