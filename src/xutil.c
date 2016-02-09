#define _DEFAULT_SOURCE

#include <stdlib.h>  /* exit(), calloc(), malloc(), EXIT_FAILURE */
#include <stdio.h>   /* fprintf() */
#include <string.h>  /* memset() */
#include <math.h>

#include "xutil.h"

unsigned int I1 = 1234;
unsigned int I2 = 5678;

extern inline double xuni_rand(void);

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
