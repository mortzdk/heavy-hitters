#ifndef H_count_median
#define H_count_median

// Standard libraries
#include <inttypes.h>
#include <stdbool.h>

// User defined libraries
#include "hash.h"
#include "sketch/sketch.h"

// Helpers
#define COUNT_MEDIAN_INDEX(width, depth, index) \
	( 2 + ( (2+(width)) * (depth) ) + (index) )

// Structures
typedef struct {
	sketch_size_t     size;    // Width and depth of sketch
	int64_t *restrict table;   // The count_median table
	int64_t *restrict median;  // A temporary table holding the potential median
	hash_t  *restrict hash;    // Structure that determedianes work of hash function
} count_median_t; 


// Initialization
count_median_t *count_median_create(hash_t *restrict hash, const uint8_t b, 
		const double epsilon, const double delta);

// Destuction
void count_median_destroy(count_median_t *restrict s);

// Update
void count_median_update(count_median_t *restrict s, const uint32_t i, 
		const int64_t c);

// Query
int64_t count_median_point(count_median_t *restrict s, const uint32_t i);
bool count_median_above_thresshold(count_median_t *restrict s,
		const uint32_t i, const uint64_t th);
int64_t count_median_range_sum(count_median_t *restrict s, const uint32_t l, 
		const uint32_t r);

// Heavy hitter thresshold
inline double count_median_heavy_hitter_thresshold(const uint64_t l1, 
		const double epsilon, const double th) {
	return th - epsilon*l1;	
}

#endif
