#ifndef H_norm
#define H_norm

#include <inttypes.h>
#include "sketch/sketch.h"

typedef void*(*n_create)(void *restrict cfg);
typedef void(*n_update)(void *restrict n, const uint32_t id,
		const int64_t value);
typedef int64_t(*n_norm)(void *restrict n);
typedef void(*n_destroy)(void *restrict n);

typedef struct {
	n_create   create;
	n_update   update;
	n_norm     norm;
	n_destroy  destroy;
} norm_func_t;

typedef struct {
	void        *restrict norm;
	norm_func_t *func;
} norm_t;

norm_t  *norm_create(void *restrict cfg, norm_func_t *restrict t);
void     norm_update(norm_t *restrict n, const uint32_t id, const int64_t value);
int64_t  norm_norm(norm_t *restrict n);
void     norm_destroy(norm_t *restrict n);

#endif
