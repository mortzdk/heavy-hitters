// Borrowed from: http://www.cs.princeton.edu/~rs/Algs3.c1-4/code.txt

#include <stdint.h>
#include "qsort.h"

void quicksort_map(uint32_t *a, int64_t l, int64_t r, uint32_t *map) { 
	int64_t i, j, k, p, q; 
	uint32_t v;
	if (r <= l) {
		return;
	}

	v = a[r];
	i = l-1;
	j = r;
	p = l-1;
	q = r;

	while (1) { 
		while (less_map(a[++i], v));

		while (less_map(v, a[--j])) {
			if (j == l) {
				break;
			}
		}

		if (i >= j) {
			break;
		}

		exch_map(a[i], a[j]);
		exch_map(map[i], map[j]);

		if (eq(a[i], v)) {
			p++;
			exch_map(a[p], a[i]);
			exch_map(map[p], map[i]);
		}

		if (eq(v, a[j])) {
			q--;
			exch_map(a[q], a[j]);
			exch_map(map[q], map[j]);
		}
	}

	exch_map(a[i], a[r]);
	exch_map(map[i], map[r]);

	j = i-1;
	i = i+1;

	for (k = l  ; k < p; k++, j--) {
		exch_map(a[k], a[j]);
		exch_map(map[k], map[j]);
	}

	for (k = r-1; k > q; k--, i++) {
		exch_map(a[k], a[i]);
		exch_map(map[k], map[i]);
	}

	quicksort_map(a, l, j, map);
	quicksort_map(a, i, r, map); 
}

void quicksort(Item a[], int64_t l, int64_t r) { 
	int64_t i, j, k, p, q; 
	Item v;
	if (r <= l) {
		return;
	}

	v = a[r];
	i = l-1;
	j = r;
	p = l-1;
	q = r;

	while (1) { 
		while (less(a[++i], v));

		while (less(v, a[--j])) {
			if (j == l) {
				break;
			}
		}

		if (i >= j) {
			break;
		}

		exch(a[i], a[j]);

		if (eq(a[i], v)) {
			p++;
			exch(a[p], a[i]);
		}

		if (eq(v, a[j])) {
			q--;
			exch(a[q], a[j]);
		}
	}

	exch(a[i], a[r]);
	j = i-1;
	i = i+1;

	for (k = l  ; k < p; k++, j--) {
		exch(a[k], a[j]);
	}

	for (k = r-1; k > q; k--, i++) {
		exch(a[k], a[i]);
	}

	quicksort(a, l, j);
	quicksort(a, i, r); 
}
