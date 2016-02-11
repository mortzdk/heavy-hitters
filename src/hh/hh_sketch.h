#ifndef H_hh_sketch
#define H_hh_sketch

// Standard libraries
#include <inttypes.h>

// User defined libraries
#include "hash.h"
#include "sketch/sketch.h"

// Structures
typedef struct {
	uint32_t *hitters;
	uint32_t count;
} heavy_hitters_t;

typedef struct {
	sketch_t      **tree;
	uint32_t       *top;
	short           top_cnt;
	uint32_t        m;
	short           logm;
	double          phi;
	double          epsilon;
	uint64_t        norm;
	heavy_hitters_t result;
} hh_sketch_t; 

// Initialization
hh_sketch_t *hh_sketch_create(sketch_func_t *f, hash_t *hash, double phi, 
		short b, double epsilon, double delta, uint32_t m);

// Destuction
void hh_sketch_destroy(hh_sketch_t *hh);

// Update
void hh_sketch_update(hh_sketch_t *hh, uint32_t idx, int32_t c);

// Query
heavy_hitters_t *hh_sketch_query(hh_sketch_t *hh);

#endif
