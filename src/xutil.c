#define _DEFAULT_SOURCE

#include <stdlib.h>  /* exit(), calloc(), malloc(), EXIT_FAILURE */
#include <stdio.h>   /* fprintf() */
#include <string.h>  /* memset() */
#include <inttypes.h>
#include <math.h>

#include "xutil.h"

uint32_t I1 = 1234;
uint32_t I2 = 5678;

extern inline uint32_t next_pow_2(uint32_t v);

extern inline double xuni_rand(void);

const short MultiplyDeBruijnBitPosition2[32] = {
	  0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 
	  31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};

void xerror(char *msg, int line, char *file) {
	fprintf(stderr, "Error %d@%s: %s\n", line, file, msg);
	exit(EXIT_FAILURE);
}

void *xcalloc(size_t nmemb, size_t size) {
	void *p;

	if (size == 0 || nmemb == 0) {
		return NULL;
	}

	p = calloc(nmemb, size);

	if (p == NULL) {
		xerror("Unable to c-allocate memory", __LINE__, __FILE__);
	}

	return p;
}

void *xmalloc(size_t size) {
	void *p;

	if (size == 0) {
		return NULL;
	}

	p = malloc(size);

	if (p == NULL) {
		xerror("Unable to m-allocate memory", __LINE__, __FILE__);
	}

	return p;
}

void *xrealloc(void *ptr, size_t size) {
	void *p;

	if (size == 0) {
		return NULL;
	}

	p = realloc(ptr, size);

	if (p == NULL) {
		xerror("Unable to r-allocate memory", __LINE__, __FILE__);
	}

	return p;
}
