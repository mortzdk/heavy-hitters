// Standard libraries
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// User defined libraries
#include "xutil.h"
#include "sketch/count_min.h"
#include "hash.h"

short M;

count_min_t *count_min_create(short b, double epsilon, double delta, hash_t *hash) {
	uint32_t i;
	count_min_t *s = xmalloc(sizeof(count_min_t));

	s->w       = ceil(b / epsilon) * hash->c;
	s->d       = ceil(log2(1 / delta) / log2(b));
	s->a       = xmalloc(sizeof(uint32_t) * s->d);
	s->b       = xmalloc(sizeof(uint32_t) * s->d);
	s->table   = xmalloc(sizeof(uint32_t) * s->d * s->w);
	s->hash    = hash;

	M = (short)floor(log2(s->w));

	memset(s->a, '\0', sizeof(uint32_t) * s->d);
	memset(s->b, '\0', sizeof(uint32_t) * s->d);
	memset(s->table, '\0', sizeof(uint32_t) * s->d * s->w);

	for (i = 0; i < s->d; i++) {
		s->a[i] = hash->agen();
		s->b[i] = hash->bgen(s->w);
	}

	return s;
}

void count_min_destroy(count_min_t *s) {
	if (s == NULL) {
		return;
	}

	if (s->a != NULL) {
		free(s->a);
		s->a = NULL;
	}

	if (s->b != NULL) {
		free(s->b);
		s->b = NULL;
	}

	if (s->table != NULL) {
		free(s->table);
		s->table = NULL;
	}

	free(s);
	s = NULL;
}

void count_min_update(count_min_t *s, uint32_t i, int32_t c) {
	uint32_t di, wi;
	uint32_t w = s->w;

	for (di = 0; di < s->d; di++) {
		wi = s->hash->hash(i, w, s->a[di], s->b[di]);

		assert( wi < w );
		/**
		 * DEBUG: 
		 *
	     *	if (wi>=w) { 
	     *		printf("name: %s\n", s->hash->name);
	     *		printf("a: %"PRIu32"\n", s->a[di]);
	     *		printf("b: %"PRIu32"\n", s->b[di]);
	     *		printf("i: %"PRIu32"\n", i);
	     *		printf("w: %"PRIu32"\n", w);
	     *		printf("wi: %"PRIu32"\n", wi);
	     *		xerror("Hmm..", __LINE__, __FILE__);
	     *	}
		*/

		s->table[COUNT_MIN_INDEX(w, di, wi)] += c;
	}
}

uint32_t count_min_point(count_min_t *s, uint32_t i) {
	uint32_t di, wi, estimate, e;
	uint32_t w = s->w;

	wi        = s->hash->hash(i, w, s->a[0], s->b[0]);
	estimate  = s->table[COUNT_MIN_INDEX(w, 0, wi)];
	for (di = 1; di < s->d; di++) {
		wi = s->hash->hash(i, w, s->a[di], s->b[di]);
		e  = s->table[COUNT_MIN_INDEX(w, di, wi)];

		if (e < estimate) {
			estimate = e;
		}
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
