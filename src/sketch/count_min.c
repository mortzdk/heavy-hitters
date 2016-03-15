// Standard libraries
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <math.h>

// User defined libraries
#include "sketch/count_min.h"
#include "sketch/sketch.h"
#include "util/hash.h"
#include "util/xutil.h"

count_min_t *count_min_create(hash_t *restrict hash, const uint8_t b, 
		const double epsilon, const double delta) {
	uint32_t i;
	count_min_t *restrict s = xmalloc(sizeof(count_min_t));
	uint32_t w              = ceil(b / epsilon) * hash->c;
	uint32_t d              = ceil(log2(1 / delta) / log2(b));

	sketch_fixed_size(&d, &w);
	hash_init(&s->size.M, w);

	const uint32_t dw       = w*d;
	const uint32_t M        = s->size.M;
	const uint32_t size     = sizeof(uint64_t) * (dw + d);

	s->table  = xmalloc(size);
	s->hash   = hash;
	s->size.w = w;
	s->size.d = d;

	memset(s->table, '\0', size);

	for (i = 0; i < d; i++) {
		s->table[i*(w+1)] |= ((uint64_t) hash->agen()) << 32;
		s->table[i*(w+1)] |= (uint64_t) hash->bgen(M);
	}

	#ifdef SPACE
	uint64_t space = sizeof(count_min_t) + size;
	fprintf(stderr, "Space usage Count-Min Sketch: %"PRIu64" bytes\n", space);
	#endif

	return s;
}

void count_min_destroy(count_min_t *restrict s) {
	if (s == NULL) {
		return;
	}

	if (s->table != NULL) {
		free(s->table);
		s->table = NULL;
	}

	free(s);
	s = NULL;
}

void count_min_update(count_min_t *restrict s, const uint32_t i, 
		const int64_t c) {
	uint32_t di, wi;
	const uint32_t w         = s->size.w;
	const uint8_t  M         = s->size.M;
	uint64_t *restrict table = s->table;
	hash hash                = s->hash->hash;                  

	for (di = 0; di < s->size.d; di++) {
		wi = hash(w, M, i, (uint32_t)(table[di*(w+1)]>>32), 
				(uint32_t)(table[di*(w+1)]));

		assert( wi < w );

		table[COUNT_MIN_INDEX(w, di, wi)] += c;
	}
}

uint64_t count_min_point(count_min_t *restrict s, const uint32_t i) {
	uint32_t di, wi;
	uint64_t estimate, e;
	const uint32_t w         = s->size.w;
	const uint8_t  M         = s->size.M;
	uint64_t *restrict table = s->table;
	hash hash                = s->hash->hash;                  

	wi = hash(w, M, i, (uint32_t)(table[0]>>32), (uint32_t)table[0]);

	assert( wi < w );

	estimate  = table[COUNT_MIN_INDEX(w, 0, wi)];
	for (di = 1; di < s->size.d; di++) {
		wi = hash(w, M, i, (uint32_t)(table[di*(w+1)]>>32), 
		          	(uint32_t)table[di*(w+1)]);

		assert( wi < w );

		e        = table[COUNT_MIN_INDEX(w, di, wi)];
		estimate = (e < estimate) ? e : estimate;
	}

	// The heavy hitter implementation does not support integer > 2^63-1
	assert( estimate < ((uint64_t)1 << 63) );

	return estimate;
}

uint64_t count_min_point_partial(count_min_t *restrict s, const uint32_t i,
		const uint32_t d) {
	(void) s;
	(void) i;
	(void) d;

	xerror("NOT IMPLEMENTED: count_min_point_partial", __LINE__, __FILE__);

	return 0;
}

bool count_min_above_thresshold(count_min_t *restrict s, const uint32_t i, 
		const uint64_t th) {
	uint32_t di, wi;
	const uint32_t w         = s->size.w;
	const uint32_t M         = s->size.M;
	uint64_t *restrict table = s->table;
	hash hash                = s->hash->hash;                  

	for (di = 0; di < s->size.d; di++) {
		wi = hash(w, M, i, (uint32_t)(table[di*(w+1)]>>32), 
		          	(uint32_t)table[di*(w+1)]);

		assert( wi < w );

		if (table[COUNT_MIN_INDEX(w, di, wi)] < th) {
			return false;
		}
	}

	return true;
}

uint64_t count_min_range_sum(count_min_t *restrict s, const uint32_t l, 
		const uint32_t r) {
	uint64_t sum = 0, i;

	for (i = l; i <= r; i++) {
		sum += count_min_point(s, i);
	}

	return sum;
}

extern inline double count_min_heavy_hitter_thresshold(const uint64_t l1, 
		const double epsilon, const double th);
