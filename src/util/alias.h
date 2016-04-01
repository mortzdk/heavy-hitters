#ifndef H_WALKER
#define H_WALKER

/**
 * This algorithm have redundancy of O(2n*log(n) + n), expected query time of 
 * O(1) and O(n) preproccessing time.
 */

#include <inttypes.h>

typedef struct {
	uint32_t  n;     // Amount of classes and probabilities
	uint32_t *alias; // The classes to sample
	double   *probs; // The probabilities to sample
} alias_t;

alias_t *alias_preprocess(uint32_t n, double *x);

int alias_draw(alias_t *restrict alias);

void alias_free(alias_t *restrict alias);

#endif
