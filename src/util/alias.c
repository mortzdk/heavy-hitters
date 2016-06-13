#include <immintrin.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "xutil.h"
#include "alias.h"

/**
 * Inspired by: http://explodecomputer.com/page.php?id=154
 *              https://github.com/wch/r-source/blob/ed66b715221d2720f5b334470335635bada520b1/src/main/random.c#L346
 **/
alias_t *alias_preprocess(uint32_t n, double *x) {
	uint32_t *HL, *H, *L, *a;
	double   *q;
	uint32_t  i, j, k;
	double    sum      = 0;
	uint32_t  intSize  = n * sizeof(uint32_t); 

	alias_t *alias = xmalloc(sizeof(alias_t));
	alias->alias   = a = xmalloc(intSize);
	alias->probs   = q = xmalloc(n * sizeof(double));
	alias->n       = n;

	HL = xmalloc(intSize);
	H  = HL - 1; 
	L  = HL + n;

    for (i = 0; i < n; i++) {
    	sum += x[i];
    }

	for (i = 0; i < n; i++) {
		x[i] /= sum;
	}

	/** 
	 * Create the alias tables.
	 * The idea is that for HL[0] ... L-1 label the entries with q < 1
	 * and L ... H[n-1] label those >= 1.
	 * By rounding error we could have q[i] < 1. or > 1. for all entries.
	 **/
	for (i = 0; i < n; i++) {
		q[i] = x[i] * n;
		if (q[i] < 1.) {
			*++H = i;
		} else {
			*--L = i;
		}
	}

	if (H >= HL && L < HL + n) { /* So some q[i] are >= 1 and some < 1 */
		for (k = 0; k < n-1; k++) {
			i = HL[k];
			j = *L;
			a[i] = j;
			q[j] += q[i] - 1;

			if (q[j] < 1.) { 
				L++;
			}

			if (L >= HL + n) {
				break; /* now all are >= 1 */
			}
		}
	}

	for (i = 0; i < n; i++) {
		q[i] += i;
	}

	free(HL);

	return alias;
}

int alias_draw(alias_t *restrict alias) {
	double rU = (double) xuni_rand() * alias->n;
	uint32_t i = (uint32_t) rU;
	
	assert(i < alias->n);

	return (rU < alias->probs[i]) ? i : alias->alias[i];
}

void alias_free(alias_t *restrict alias) {
	if (NULL != alias) {
		if (NULL != alias->alias) {
			free(alias->alias);
		}

		if (NULL != alias->probs) {
			free(alias->probs);
		}

		free(alias);
	}
}
