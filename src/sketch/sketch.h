#ifndef H_sketch
#define H_sketch

// Standard libraries
#include <inttypes.h>

// User defined libraries
#include "sketch/count_min.h"

typedef void*(*create)(short b, double epsilon, double delta, hash_t *hash);
typedef void(*update)(void *s, uint32_t i, int32_t c);
typedef uint32_t(*point)(void *s, uint32_t i);
typedef uint32_t(*rangesum)(void *s, uint32_t l, uint32_t r);
typedef void(*destroy)(void *s);

typedef struct {
	create   create;
	destroy  destroy;
	update   update;
	point    point;
	rangesum rangesum;
} sketch_func_t;

typedef struct {
	void          *sketch;
	sketch_func_t *funcs;
} sketch_t;

sketch_t *sketch_create(sketch_func_t *f, short b, double epsilon, double delta, hash_t *hash);
void      sketch_destroy(sketch_t *s);
void      sketch_update(sketch_t *s, uint32_t i, int32_t c);
uint32_t  sketch_point(sketch_t *s, uint32_t i);
uint32_t  sketch_range_sum(sketch_t *s, uint32_t l, uint32_t r);

/**
 * Structures holding function pointers for different sketch implementations
 */
extern sketch_func_t countMin;
extern sketch_func_t countMedian;

#endif
