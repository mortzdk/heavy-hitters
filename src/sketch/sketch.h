#ifndef H_sketch
#define H_sketch

// Standard libraries
#include <stdint.h>

// User defined libraries
#include "hash.h"

typedef void*(*s_create)(hash_t *hash, uint8_t b, double epsilon, double delta);
typedef void(*s_destroy)(void *s);
typedef void(*s_update)(void *s, uint32_t i, int32_t c);
typedef uint32_t(*s_point)(void *s, uint32_t i);
typedef uint32_t(*s_rangesum)(void *s, uint32_t l, uint32_t r);

typedef struct {
	uint32_t w;
	uint32_t d;
} sketch_size_t;

typedef struct {
	s_create   create;
	s_destroy  destroy;
	s_update   update;
	s_point    point;
	s_rangesum rangesum;
} sketch_func_t;

typedef struct {
	void          *sketch;
	sketch_func_t *funcs;
} sketch_t;

inline uint32_t sketch_depth(void *sketch) {
	return ((sketch_size_t *)sketch)->d;
}

inline uint32_t sketch_width(void *sketch) {
	return ((sketch_size_t *)sketch)->w;
}

sketch_t *sketch_create(sketch_func_t *f, hash_t *hash, uint8_t b, 
		double epsilon, double delta);
void      sketch_destroy(sketch_t *s);
void      sketch_update(sketch_t *s, uint32_t i, int64_t c);
uint32_t  sketch_point(sketch_t *s, uint32_t i);
uint32_t  sketch_range_sum(sketch_t *s, uint32_t l, uint32_t r);

/**
 * Structures holding function pointers for different sketch implementations
 */
extern sketch_func_t countMin;
extern sketch_func_t countMedian;

#endif
