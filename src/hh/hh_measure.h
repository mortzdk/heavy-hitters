#ifndef H_hh_measure
#define H_hh_measure

#include <stdint.h>

// User defined libraries
#include "util/hash.h"
#include "hh/hh.h"

typedef struct {
	void      *restrict hh;	
	hh_func_t *restrict funcs;	
	uint32_t i;
	uint32_t c;
} hh_measure_t;

hh_measure_t *heavy_hitter_measure_create(heavy_hitter_params_t *restrict params);

// Destuction
void heavy_hitter_measure_destroy(hh_measure_t *restrict params);

// Update
void heavy_hitter_measure_update(hh_measure_t *restrict params);

// Query
heavy_hitter_t *heavy_hitter_measure_query(hh_measure_t *restrict params);

#endif
