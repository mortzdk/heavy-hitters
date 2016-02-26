// Standard libraries
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <math.h>

// User defined libraries
#include "xutil.h"
#include "sketch/count_median.h"
#include "hash.h"

/* http://ndevilla.free.fr/median/median/index.html */

/*
 *  This Quickselect routine is based on the algorithm described in
 *  "Numerical recipes in C", Second Edition,
 *  Cambridge University Press, 1992, Section 8.5, ISBN 0-521-43108-5
 *  This code by Nicolas Devillard - 1998. Public domain.
 */

#define SWAP(a, b) { register int64_t t=(a);(a)=(b);(b)=t; }

static int64_t quick_select(int64_t *restrict v, const uint32_t n) {
    register uint32_t middle, ll, hh;
    register uint32_t low          = 0;
	register uint32_t high         = n-1;
    register const uint32_t median = (low + high) / 2;

    for (;;) {
        if ( unlikely(high <= low) ) {
			/* One element only */
            return v[median];
		}

        if ( unlikely(high == low + 1) ) {
			/* Two elements only */
            if ( v[low] > v[high] ) {
                SWAP(v[low], v[high]);
			}
            return v[median] ;
        }

		/* Find median of low, middle and high items; swap into position low */
		middle = (low + high) / 2;

		if ( v[middle] > v[high] ) {
			SWAP(v[middle], v[high]);
		}

		if ( v[low] > v[high] ) {
			SWAP(v[low], v[high]);
		}

		if ( v[middle] > v[low] ) {
			SWAP(v[middle], v[low]);
		}

		/* Swap low item (now in position middle) into position (low+1) */
		SWAP(v[middle], v[low+1]);

		/* Nibble from each end towards middle, swapping items when stuck */
		ll = low + 1;
		hh = high;

		for (;;) {
			do {
				ll++;
			} while (v[low] > v[ll]);
			do {
				hh--;
			} while (v[hh]  > v[low]);

			if (hh < ll) {
				break;
			}

			SWAP(v[ll], v[hh]) ;
		}

		/* Swap middle item (in position low) back into correct position */
		SWAP(v[low], v[hh]) ;

		/* Re-set active partition */
		if ( hh <= median ) {
			low  = ll;
		}
		if ( hh >= median ) {
			high = hh - 1;
		}
    }
}

/*
 * Algorithm from N. Wirth's book, implementation by N. Devillard.
 * This code in public domain.
 */

/*---------------------------------------------------------------------------
   Function :   wirth()
   In       :   array of elements, # of elements in the array
   Out      :   one element
   Job      :   find the median element in the array

                Reference:

                  Author: Wirth, Niklaus 
                   Title: Algorithms + data structures = programs 
               Publisher: Englewood Cliffs: Prentice-Hall, 1976 
    Physical description: 366 p. 
                  Series: Prentice-Hall Series in Automatic Computation 

 ---------------------------------------------------------------------------*/

static int64_t wirth(int64_t *restrict v, const uint32_t n) {
    register uint32_t i, j, l, m;
    register int64_t x;
	register const uint32_t k = (n-1)/2;

    l = 0;
	m = n-1;
    while (l < m) {
        x = v[k];
        i = l;
        j = m;
        do {
            while ( v[i] < x ) {
				i++;
			}
            while ( x < v[j] ) {
				j--;
			}
            if ( i <= j ) {
                SWAP(v[i], v[j]);
                i++;
				j--;
            }
        } while ( i <= j );
        if ( j < k ) {
			l = i;
		}
        if ( k < i ) {
			m = j;
		}
    }
    return v[k];
}

/*****************************************************************************
 *                     Count-Median Sketch Data Structure                    *
 *****************************************************************************/

count_median_t *count_median_create(hash_t *restrict hash, const uint8_t b, 
		const double epsilon, const double delta) {
	register uint32_t i;
	count_median_t *restrict s = xmalloc(sizeof(count_median_t));
	register const uint32_t w  = s->size.w = ceil(b / (epsilon*epsilon)) * hash->c;
	register const uint32_t d  = s->size.d = ceil( log((1./delta)) * 
			((double)((uint32_t)(b-1)*b << 3)/pow(b-2, 2)));
	hash_init(&s->size.M, w);
	const uint32_t table_size  = sizeof(int64_t) * ((w+2)*d);
	const uint32_t median_size = sizeof(int64_t) * d;

	assert( b > 2 );

	s->table   = xmalloc(table_size);
	s->median  = xmalloc(median_size);
	s->hash    = hash;

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
	fprintf(strerr, "Space usage Count-Median Sketch: %"PRIu64" bytes\n", space);
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
	const uint32_t w = s->size.w;
	const uint8_t  M = s->size.M;

	for (di = 0; di < s->size.d; di++) {
		wi = s->hash->hash(w, M, i, (uint32_t)(s->table[di*(w+2)]>>32), 
				(uint32_t)(s->table[di*(w+2)]));

		assert( wi < w );

		s->table[COUNT_MEDIAN_INDEX(w, di, wi)] += c * sign(i, 
				(uint8_t)(s->table[di*(w+2)+1]>>32), 
				(uint8_t)(s->table[di*(w+2)+1]));
	}
}

int64_t count_median_point(count_median_t *restrict s, const uint32_t i) {
	uint32_t di, wi;
	const uint32_t d = s->size.d;
	const uint32_t w = s->size.w;
	const uint8_t  M = s->size.M;

	for (di = 0; di < d; di++) {
		wi = s->hash->hash(w, M, i, (uint32_t)(s->table[di*(w+2)]>>32), 
		          	(uint32_t)s->table[di*(w+2)]);

		assert( wi < w );

		s->median[di] = s->table[COUNT_MEDIAN_INDEX(w, di, wi)] * sign(i, 
				(uint8_t)(s->table[di*(w+2)+1]>>32), 
				(uint8_t)(s->table[di*(w+2)+1]));
	}

//	return quick_select(s->median, d);
	return wirth(s->median, d);
}

int64_t count_median_range_sum(count_median_t *restrict s, const uint32_t l, 
		const uint32_t r) {
	int64_t sum = 0, i;

	for (i = l; i <= r; i++) {
		sum += count_median_point(s, i);
	}

	return sum;
}

extern inline double count_median_heavy_hitter_thresshold(uint64_t l1, 
		double epsilon, double th);
