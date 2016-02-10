#ifndef H_sketch
#define H_sketch

#include <inttypes.h>

#include "hash.h"

// Helpers
#define SKETCH_INDEX(w, di, wi) ( ( w * di ) + wi )


// Structures
typedef struct {
	uint32_t  w;
	uint32_t  d;
	uint32_t *a;
	uint32_t *b;
	uint32_t *table;
	hash_t   *hash;
} sketch_t; 


// Initialization
sketch_t *sketch_create(int b, double epsilon, double delta, hash_t *hash);


// Destuction
void sketch_destroy(sketch_t *s);


// Update
void sketch_update(sketch_t *s, uint32_t i, int32_t c);


// Query
uint32_t sketch_point(sketch_t *s, uint32_t i);

uint32_t sketch_range(sketch_t *s, uint32_t l, uint32_t r);

uint32_t sketch_range_naive(sketch_t *s, uint32_t l, uint32_t r);

#endif
