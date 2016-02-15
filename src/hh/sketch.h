#ifndef H_hh_sketch
#define H_hh_sketch

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
	sketch_func_t  *f;
} hh_sketch_params_t;

typedef struct {
	sketch_t             **tree;
	uint32_t              *top;
	uint8_t                top_cnt;
	uint8_t                logm;
	uint64_t               norm;
	hh_sketch_params_t    *params;
	heavy_hitter_t         result;
} hh_sketch_t; 

// Initialization
hh_sketch_t *hh_sketch_create(heavy_hitter_params_t *p);

// Destuction
void hh_sketch_destroy(hh_sketch_t *hh);

// Update
void hh_sketch_update(hh_sketch_t *hh, uint32_t idx, int32_t c);

// Query
heavy_hitter_t *hh_sketch_query(hh_sketch_t *hh);

#endif
