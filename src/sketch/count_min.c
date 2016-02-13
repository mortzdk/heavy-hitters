// Standard libraries
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

// User defined libraries
#include "xutil.h"
#include "sketch/count_min.h"
#include "hash.h"

count_min_t *count_min_create(hash_t *hash, uint8_t b, double epsilon, 
		double delta) {
	uint32_t i, w, d, dw;
	count_min_t *s = xmalloc(sizeof(count_min_t));

	w        = s->size.w = ceil(b / epsilon) * hash->c;
	d        = s->size.d = ceil(log2(1 / delta) / log2(b));
	dw       = w*d;

	s->table = xmalloc(sizeof(uint64_t) * (dw + d));
	s->hash  = hash;

	memset(s->table, '\0', sizeof(uint64_t) * (dw + d));

	for (i = 0; i < d; i++) {
		s->table[i*(w+1)] |= ((uint64_t) hash->agen()) << 32;
		s->table[i*(w+1)] |= (uint64_t) hash->bgen(w);
	}

	return s;
}

void count_min_destroy(count_min_t *s) {
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

void count_min_update(count_min_t *s, uint32_t i, int64_t c) {
	uint32_t di, wi;
	uint32_t w = s->size.w;

	for (di = 0; di < s->size.d; di++) {
		wi = s->hash->hash(i, w, (uint32_t)(s->table[di*(w+1)]>>32), 
				(uint32_t)(s->table[di*(w+1)]));

		assert( wi < w );

		s->table[COUNT_MIN_INDEX(w, di, wi)] += c;
	}
}

uint32_t count_min_point(count_min_t *s, uint32_t i) {
	uint32_t di, wi, estimate, e;
	uint32_t w = s->size.w;

	wi        = s->hash->hash(i, w, (uint32_t)(s->table[0]>>32), 
			(uint32_t)s->table[0]);

	assert( wi < w );

	estimate  = s->table[COUNT_MIN_INDEX(w, 0, wi)];
	for (di = 1; di < s->size.d; di++) {
		wi       = s->hash->hash(i, w, (uint32_t)(s->table[di*(w+1)]>>32), 
		          	(uint32_t)s->table[di*(w+1)]);

		assert( wi < w );

		e        = s->table[COUNT_MIN_INDEX(w, di, wi)];
		estimate = (e < estimate) ? e : estimate;
	}

	return estimate;
}

uint32_t count_min_range_sum(count_min_t *s, uint32_t l, uint32_t r) {
	uint32_t sum = 0, i;

	for (i = l; i <= r; i++) {
		sum += count_min_point(s, i);
	}

	return sum;
}
