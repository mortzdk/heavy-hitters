#ifndef H_sketch
#define H_sketch

// Standard libraries
#include <stdint.h>
#include <stdbool.h>

// User defined libraries
#include "util/hash.h"

typedef void*(*s_create)(hash_t *restrict hash, const uint8_t b, 
		const double epsilon, const double delta);
typedef void(*s_destroy)(void *restrict s);
typedef void(*s_update)(void *restrict s, const uint64_t i, const int64_t c);
typedef uint64_t(*s_point)(void *restrict s, const uint32_t i);
typedef uint64_t(*s_point_partial)(void *restrict s, const uint32_t i,
		const uint32_t d);
typedef bool(*s_above)(void *restrict s, const uint32_t i, const uint64_t th);
typedef uint64_t(*s_rangesum)(void *restrict s, const uint32_t l, 
		const uint32_t r);
typedef double (*s_thresshold)(uint64_t l1, double epsilon, double th);

typedef struct {
	uint32_t w;
	uint32_t d;
	uint8_t  M;
} sketch_size_t;

typedef struct {
	s_create        create;
	s_destroy       destroy;
	s_update        update;
	s_point         point;
	s_point_partial point_partial;
	s_above         above;
	s_rangesum      rangesum;
	s_thresshold    thresshold;
} sketch_func_t;

typedef struct {
	void          *restrict sketch;
	sketch_func_t *restrict funcs;
} sketch_t;

extern uint32_t depth;
extern uint32_t width;

inline void sketch_fixed_size(uint32_t *restrict d, uint32_t *restrict w){
	if ( depth > 0 ) {
		*d = depth;
	}

	if ( width > 0 ) {
		*w = width;
	}
}

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
int64_t  sketch_point(sketch_t *restrict s, const uint32_t i);
int64_t  sketch_point_partial(sketch_t *restrict s, const uint32_t i,
		const uint32_t d);
bool      sketch_above_thresshold(sketch_t *restrict s, const uint32_t i, 
		const uint64_t th);
int64_t  sketch_range_sum(sketch_t *restrict s, const uint32_t l, 
		const uint32_t r);
double sketch_thresshold(sketch_t *restrict s, const uint64_t l1, 
		const double epsilon, const double th);

/**
 * Structures holding function pointers for different sketch implementations
 */
extern sketch_func_t countMin;
extern sketch_func_t countMedian;

#endif
