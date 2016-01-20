#ifndef H_XUTIL
#define H_XUTIL

#ifndef BYTE
#define BYTE 8
#endif

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

#include <stdlib.h>  /* random */

extern unsigned int I1, I2;

inline double xuni_rand(void) {
	I1 = 36969*(I1 & 0177777) + (I1>>16);
	I2 = 18000*(I2 & 0177777) + (I2>>16);
	return ((I1 << 16)^(I2 & 0177777)) * 2.328306437080797e-10; /* in [0,1) */
}

void xerror(char *msg, int line, char *file);

void *xcalloc(size_t nmemb, size_t size);

void *xmalloc(size_t size); 

void *xrealloc(void *ptr, size_t size);

#endif
