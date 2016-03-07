#ifndef H_sketch_measure
#define H_sketch_measure

// Standard libraries
#include <stdint.h>
#include <stdbool.h>

// User defined libraries
#include "hash.h"
#include "sketch/sketch.h"

typedef struct {
	void          *restrict sketch;
	sketch_func_t *restrict funcs;
} sketch_measure_t;

typedef struct {
	sketch_func_t *restrict f;
	hash_t *restrict hash;
	const uint8_t b;
	const double epsilon;
	const double delta;
} sketch_measure_create_t;

typedef struct {
	sketch_measure_t *s;
	const uint32_t i;
	const int64_t  c;
} sketch_measure_params_t;

sketch_measure_t *sketch_measure_create(sketch_measure_create_t *params);
void              sketch_measure_destroy(sketch_measure_params_t *params);
void              sketch_measure_update(sketch_measure_params_t *params);
int64_t           sketch_measure_point(sketch_measure_params_t *params);

#endif
