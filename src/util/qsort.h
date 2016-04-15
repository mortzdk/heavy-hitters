// Borrowed from: http://www.cs.princeton.edu/~rs/Algs3.c1-4/code.txt

#ifndef qsort_h
#define qsort_h

#include <stdint.h>

typedef uint64_t Item;

#define key(A) (A)
#define less(A, B) (key(A) < key(B))
#define exch(A, B) { Item t = A; A = B; B = t; } 
#define eq(A, B) (!less(A, B) && !less(B, A))

void quicksort(Item a[], int64_t l, int64_t r);

#endif
