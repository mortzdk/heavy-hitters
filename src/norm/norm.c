#include "norm/norm.h"

norm_t *norm_create(void *restrict cfg, norm_func_t *restrict f) {
	norm_t *restrict n = xmalloc(sizeof(norm_t));
	n->func 		   = f;
	n->norm  		   = n->func->create(cfg);

	return n;
}

void norm_update(norm_t *n, uint32_t id, int64_t value) {
	n->func->update(n->norm, id, value);
}

int64_t norm_norm(norm_t *n) {
	return n->func->norm(n->norm);
}

void norm_destroy(norm_t *n) {
	if (n == NULL) {
		return;
	}

	n->func->destroy(n->norm);

	free(n);
}
