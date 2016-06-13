// Borrowed from: http://www.cs.princeton.edu/~rs/Algs3.c1-4/code.txt

#ifndef qsort_h
#define qsort_h

#include <stdint.h>

typedef uint64_t Item;

#define key(A) (A)
#define less_map(A, B) (key(A) > key(B))
#define less(A, B) (key(A) < key(B))
#define exch(A, B) { Item t = A; A = B; B = t; } 
#define exch_map(A, B) { uint32_t t = A; A = B; B = t; } 
#define eq(A, B) (!less(A, B) && !less(B, A))

void quicksort(Item a[], int64_t l, int64_t r);
void quicksort_map(uint32_t *a, int64_t l, int64_t r, uint32_t *map);

#endif
