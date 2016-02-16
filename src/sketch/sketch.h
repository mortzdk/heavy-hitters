#ifndef H_sketch
#define H_sketch

// Standard libraries
#include <stdint.h>
#include <stdbool.h>

// User defined libraries
#include "hash.h"

typedef void*(*s_create)(hash_t *restrict hash, const uint8_t b, 
		const double epsilon, const double delta);
typedef void(*s_destroy)(void *restrict s);
typedef void(*s_update)(void *restrict s, const uint64_t i, const int64_t c);
typedef uint64_t(*s_point)(void *restrict s, const uint32_t i);
typedef bool(*s_above)(void *restrict s, const uint32_t i, const uint64_t th);
typedef uint64_t(*s_rangesum)(void *restrict s, const uint32_t l, 
		const uint32_t r);

typedef struct {
	uint32_t w;
	uint32_t d;
} sketch_size_t;

typedef struct {
	s_create   create;
	s_destroy  destroy;
	s_update   update;
	s_point    point;
	s_above    above;
	s_rangesum rangesum;
} sketch_func_t;

typedef struct {
	void          *restrict sketch;
	sketch_func_t *restrict funcs;
} sketch_t;

inline uint32_t sketch_depth(void *restrict sketch) {
	return ((sketch_size_t *)sketch)->d;
}

inline uint32_t sketch_width(void *restrict sketch) {
	return ((sketch_size_t *)sketch)->w;
}

sketch_t *sketch_create(sketch_func_t *restrict f, hash_t *restrict hash, 
		const uint8_t b, const double epsilon, const double delta);
void      sketch_destroy(sketch_t *restrict s);
void      sketch_update(sketch_t *restrict s, const uint32_t i, 
		const int64_t c);
uint64_t  sketch_point(sketch_t *restrict s, const uint32_t i);
bool      sketch_above_thresshold(sketch_t *restrict s, const uint32_t i, 
		const uint64_t th);
uint64_t  sketch_range_sum(sketch_t *restrict s, const uint32_t l, 
		const uint32_t r);

/**
 * Structures holding function pointers for different sketch implementations
 */
extern sketch_func_t countMin;
extern sketch_func_t countMedian;

#endif
