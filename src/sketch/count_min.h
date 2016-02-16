#ifndef H_count_min
#define H_count_min

// Standard libraries
#include <inttypes.h>
#include <stdbool.h>

// User defined libraries
#include "hash.h"
#include "sketch/sketch.h"

// Helpers
#define COUNT_MIN_INDEX(width, depth, index) \
	( 1 + ( (1+width) * depth ) + index )

// Structures
typedef struct {
	sketch_size_t size;  // Width and depth of sketch
	uint64_t     *table; // The count_min table
	hash_t       *hash;  // Structure that determines work of hash function
} count_min_t; 


// Initialization
count_min_t *count_min_create(hash_t *hash, uint8_t b, double epsilon, double delta);

// Destuction
void count_min_destroy(count_min_t *s);

// Update
void count_min_update(count_min_t *s, uint32_t i, int64_t c);

// Query
uint64_t count_min_point(count_min_t *s, uint32_t i);
bool count_min_above_thresshold(count_min_t *s, uint32_t i, uint64_t th);
uint64_t count_min_range_sum(count_min_t *s, uint32_t l, uint32_t r);

#endif
