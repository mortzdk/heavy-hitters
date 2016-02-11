#ifndef H_count_min
#define H_count_min

// Standard libraries
#include <inttypes.h>

// User defined libraries
#include "hash.h"
#include "sketch/sketch.h"

// Helpers
#define COUNT_MIN_INDEX(w, di, wi) ( ( w * di ) + wi )

// Structures
typedef struct {
	sketch_size_t size; // Width and depth of sketch
	/*
	uint32_t  w;     // Width of the count_min table
	uint32_t  d;     // Depth of the count_min table
	*/
	uint32_t *a;     // Array of a values for the d hash functions
	uint32_t *b;     // Array of b values for the d hash functions
	uint32_t *table; // The count_min table
	hash_t   *hash;  // Structure that determines work of hash function
} count_min_t; 


// Initialization
count_min_t *count_min_create(short b, double epsilon, double delta, hash_t *hash);

// Destuction
void count_min_destroy(count_min_t *s);

// Update
void count_min_update(count_min_t *s, uint32_t i, int32_t c);

// Query
uint32_t count_min_point(count_min_t *s, uint32_t i);
uint32_t count_min_range_sum(count_min_t *s, uint32_t l, uint32_t r);

#endif
