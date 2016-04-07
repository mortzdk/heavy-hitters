#ifndef H_norm_l2_sketch
#define H_norm_l2_sketch

#include <inttypes.h>

#include "sketch/sketch.h"
#include "norm/norm.h"


typedef struct {
	sketch_t *sketch;
	int64_t  *sum;
	int64_t  *median;
} l2_sketch_t;

typedef struct {
	sketch_t *sketch;
} l2_sketch_config;


l2_sketch_t *l2_sketch_create(l2_sketch_config *cfg);

void l2_sketch_update(l2_sketch_t *l2_sketch, uint32_t id, int64_t value);

int64_t l2_sketch_norm(l2_sketch_t *l2_sketch);

void l2_sketch_destroy(l2_sketch_t *l2_sketch);

/**
 * Structures holding function pointers for different norm implementations
 */
extern norm_func_t norm_func_l2_sketch;

#endif
