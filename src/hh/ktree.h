#ifndef H_hh_ktree
#define H_hh_ktree

// Standard libraries
#include <stdint.h>

// User defined libraries
#include "util/hash.h"
#include "util/fifo.h"
#include "hh/hh.h"
#include "sketch/sketch.h"

// Structures
typedef struct {
	double          phi;
	double          epsilon;
	double          delta;
	uint32_t        m;
	uint32_t        b;
	uint8_t         gran;
	sketch_func_t  *restrict f;
} hh_ktree_params_t;

typedef struct {
	sketch_t             **restrict tree;
	uint64_t              *restrict top;
	uint8_t                top_cnt;
	uint8_t                logm;
	uint8_t                gran;
	uint32_t               k;
	uint64_t               norm;
	hh_ktree_params_t     *restrict params;
	fifo_t                *restrict fifo;
	heavy_hitter_t         result;
} hh_ktree_t; 

// Initialization
hh_ktree_t *hh_ktree_create(heavy_hitter_params_t *restrict p);

// Destuction
void hh_ktree_destroy(hh_ktree_t *restrict hh);

// Update
void hh_ktree_update(hh_ktree_t *restrict hh, const uint32_t idx, 
		const int64_t c);

// Query
heavy_hitter_t *hh_ktree_query(hh_ktree_t *restrict hh);
heavy_hitter_t *hh_ktree_query_recursive(hh_ktree_t *restrict hh);

#endif
