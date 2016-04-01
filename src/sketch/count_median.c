// Standard libraries
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <math.h>

// User defined libraries
#include "sketch/count_median.h"
#include "sketch/sketch.h"
#include "util/xutil.h"
#include "util/hash.h"
#include "util/median.h"

/*****************************************************************************
 *                     Count-Median Sketch Data Structure                    *
 *****************************************************************************/

count_median_t *count_median_create(hash_t *restrict hash, const uint8_t b, 
		const double epsilon, const double delta) {
	register uint32_t i;
	count_median_t *restrict s = xmalloc(sizeof(count_median_t));
	uint32_t w                 = ceil(b / (epsilon*epsilon)) * hash->c;
	uint32_t d                 = ceil( log((1./delta))); //6*(log(1/delta)+1);

	sketch_fix_size(&d, &w);
	hash_init(&s->size.M, w);

	const uint32_t table_size  = sizeof(int64_t) * ((w+2)*d);
	const uint32_t median_size = sizeof(int64_t) * d;

	assert( b > 2 );

	s->table   = xmalloc(table_size);
	s->median  = xmalloc(median_size);
	s->hash    = hash;
	s->size.w  = w;
	s->size.d  = d;

	memset(s->table,  '\0', table_size);
	memset(s->median, '\0', median_size);

	for (i = 0; i < d; i++) {
		s->table[i*(w+2)]   |= ((uint64_t) hash->agen()) << 32;
		s->table[i*(w+2)]   |= (uint64_t) hash->bgen(s->size.M);
		s->table[i*(w+2)+1] |= ((uint64_t) sign_agen()) << 32;
		s->table[i*(w+2)+1] |= (uint64_t) sign_bgen();
	}

	#ifdef SPACE
	uint64_t space = sizeof(count_median_t) + table_size + median_size;
	fprintf(stderr, "Space usage Count-Median Sketch: %"PRIu64" bytes\n", space);
	#endif

	return s;
}

void count_median_destroy(count_median_t *restrict s) {
	if (s == NULL) {
		return;
	}

	if (s->median != NULL) {
		free(s->median);
		s->median = NULL;
	}

	if (s->table != NULL) {
		free(s->table);
		s->table = NULL;
	}

	free(s);
	s = NULL;
}

void count_median_update(count_median_t *restrict s, const uint32_t i, 
		const int64_t c) {
	int64_t wi;
	uint32_t di;
	const uint32_t d        = s->size.d;
	const uint32_t w        = s->size.w;
	const uint8_t  M        = s->size.M;
	int64_t *restrict table = s->table;
	hash hash               = s->hash->hash;

	for (di = 0; di < d; di++) {
		wi = hash(w, M, i, (uint32_t)(table[di*(w+2)]>>32), 
				(uint32_t)(table[di*(w+2)]));

		assert( wi < w );

		table[COUNT_MEDIAN_INDEX(w, di, wi)] += c * sign(i, 
				(uint8_t)(table[di*(w+2)+1]>>32), 
				(uint8_t)(table[di*(w+2)+1]));
	}
}

int64_t count_median_point(count_median_t *restrict s, const uint32_t i) {
	uint32_t di, wi;
	const uint32_t d         = s->size.d;
	const uint32_t w         = s->size.w;
	const uint8_t  M         = s->size.M;
	int64_t *restrict table  = s->table;
	int64_t *restrict median = s->median;
	hash hash                = s->hash->hash;

	for (di = 0; di < d; di++) {
		wi = hash(w, M, i, (uint32_t)(table[di*(w+2)]>>32), 
		          	(uint32_t)table[di*(w+2)]);

		assert( wi < w );

		median[di] = table[COUNT_MEDIAN_INDEX(w, di, wi)] * sign(i, 
				(uint8_t)(table[di*(w+2)+1]>>32), 
				(uint8_t)(table[di*(w+2)+1]));
	}

//	return median_quick_select(median, d);
	return median_wirth(median, d);
}

int64_t count_median_range_sum(count_median_t *restrict s, const uint32_t l, 
		const uint32_t r) {
	int64_t sum = 0, i;

	for (i = l; i <= r; i++) {
		sum += count_median_point(s, i);
	}

	return sum;
}

extern inline double count_median_heavy_hitter_thresshold( const uint64_t l1, 
		const double epsilon, const double th);
