#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "sketch.h"
#include "xutil.h"

sketch_t *sketch_create(int b, double epsilon, double delta) {
	uint32_t i;
	sketch_t *s = xmalloc(sizeof(sketch_t));

	s->w = ceil(b / epsilon);

	// TODO: because of hash function
	s->w = s->w * 2;

	s->d = ceil(log2(1 / delta) / log2(b));

	s->a = xmalloc(sizeof(uint32_t) * s->d);
	s->b = xmalloc(sizeof(uint32_t) * s->d);
	s->table = xmalloc(sizeof(uint32_t) * s->d * s->w);

	memset(s->a, '\0', sizeof(uint32_t) * s->d);
	memset(s->b, '\0', sizeof(uint32_t) * s->d);
	memset(s->table, '\0', sizeof(uint32_t) * s->d * s->w);

	for (i = 0; i < s->d; i++) {
		s->a[i] = 1 + (xuni_rand() * (MOD_P - 2));
		s->b[i] = (xuni_rand() * (MOD_P - 1));
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

uint32_t sketch_hash(uint32_t i, uint32_t w, uint32_t a, uint32_t b) {
	uint64_t result;
	uint32_t result32;

	result = (a * i) + b;
	result = ( (result >> 31) + result ) & MOD_P;

	result32 = (uint32_t) result;

	// TODO: and'e me w-1 når w er power af 2
	return result32 % w;

//	result = ( ( (a * i) + b ) % MOD_P) % w;
//
//	return result;
}

void sketch_update(sketch_t *s, uint32_t i, int32_t c) {
	uint32_t di, wi;
	uint32_t w = s->w;

	for (di = 0; di < s->d; di++) {
		wi = sketch_hash(i, w, s->a[di], s->b[di]);

		if (wi>=16) { 
			printf("%"PRIu32"\n", wi);
			xerror("Hmm..", __LINE__, __FILE__);
		}

		s->table[SKETCH_INDEX(w, di, wi)] += c;
	}
}

uint32_t sketch_point(sketch_t *s, uint32_t i) {
	uint32_t di, wi, estimate = UINT32_MAX, e;
	uint32_t w = s->w;

	for (di = 0; di < s->d; di++) {
		wi = sketch_hash(i, w, s->a[di], s->b[di]);
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
