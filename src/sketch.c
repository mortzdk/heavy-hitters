#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "xutil.h"
#include "hash.h"
#include "sketch.h"

sketch_t *sketch_create(int b, double epsilon, double delta, hash_t *hash) {
	uint32_t i;
	sketch_t *s = xmalloc(sizeof(sketch_t));

	s->w = ceil(b / epsilon);

	s->w = s->w * hash->c;

	s->d = ceil(log2(1 / delta) / log2(b));

	s->a = xmalloc(sizeof(uint32_t) * s->d);
	s->b = xmalloc(sizeof(uint32_t) * s->d);
	s->table = xmalloc(sizeof(uint32_t) * s->d * s->w);
	s->hash = hash;

	memset(s->a, '\0', sizeof(uint32_t) * s->d);
	memset(s->b, '\0', sizeof(uint32_t) * s->d);
	memset(s->table, '\0', sizeof(uint32_t) * s->d * s->w);

	for (i = 0; i < s->d; i++) {
		s->a[i] = hash->agen();
		s->b[i] = hash->bgen(s->w);
	}

	return s;
}

void sketch_destroy(sketch_t *s) {
	if (s == NULL) {
		return;
	}

	free(s->a);
	free(s->b);
	free(s->table);

	free(s);
	s = NULL;
}

void sketch_update(sketch_t *s, uint32_t i, int32_t c) {
	uint32_t di, wi;
	uint32_t w = s->w;

	for (di = 0; di < s->d; di++) {
		wi = s->hash->hash(i, w, s->a[di], s->b[di]);

		assert( wi < w );
		/*
		if (wi>=w) { 
			printf("name: %s\n", s->hash->name);
			printf("a: %"PRIu32"\n", s->a[di]);
			printf("b: %"PRIu32"\n", s->b[di]);
			printf("i: %"PRIu32"\n", i);
			printf("w: %"PRIu32"\n", w);
			printf("wi: %"PRIu32"\n", wi);
			xerror("Hmm..", __LINE__, __FILE__);
		}
		*/

		s->table[SKETCH_INDEX(w, di, wi)] += c;
	}
}

uint32_t sketch_point(sketch_t *s, uint32_t i) {
	uint32_t di, wi, estimate = UINT32_MAX, e;
	uint32_t w = s->w;

	for (di = 0; di < s->d; di++) {
		wi = s->hash->hash(i, w, s->a[di], s->b[di]);
		e = s->table[SKETCH_INDEX(w, di, wi)];

		if (e < estimate) {
			estimate = e;
		}
	}

	return estimate;
}

uint32_t sketch_range_naive(sketch_t *s, uint32_t l, uint32_t r) {
	uint32_t sum = 0, i;

	for (i = l; i <= r; i++) {
		sum += sketch_point(s, i);
	}

	return sum;
}
