#include <inttypes.h>

#include "median.h"
#include "xutil.h"

int64_t median_quick_select(int64_t *restrict v, const uint32_t n) {
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

int64_t median_wirth(int64_t *restrict v, const uint32_t n) {
    register int32_t i, j, l, m;
    register int64_t x;
	register const int32_t k = (n-1)/2;

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
