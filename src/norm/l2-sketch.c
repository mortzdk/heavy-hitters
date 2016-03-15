#include <math.h>
#include <string.h>

#include "norm/l2-sketch.h"
#include "norm/norm.h"
#include "util/median.h"
#include "util/xutil.h"

norm_func_t norm_func_l2_sketch = {
	.create     = (n_create)       l2_sketch_create,
	.update     = (n_update)       l2_sketch_update,
	.norm       = (n_norm)         l2_sketch_norm,
	.destroy    = (n_destroy)      l2_sketch_destroy,
};

l2_sketch_t *l2_sketch_create(l2_sketch_config *cfg) {
	uint32_t i;

	l2_sketch_t *l2_sketch = xmalloc(sizeof(l2_sketch_t));

	l2_sketch->sketch = cfg->sketch;

	uint32_t d = sketch_depth(cfg->sketch->sketch);
	l2_sketch->sum    = xmalloc(d * sizeof(int64_t));
	l2_sketch->median = xmalloc(d * sizeof(int64_t));

	for (i = 0; i < d; i++) {
		l2_sketch->sum[i] = 0;
		l2_sketch->median[i] = 0;
	}

	return l2_sketch;
}

void l2_sketch_update(l2_sketch_t *l2_sketch, uint32_t id, int64_t value) {
	sketch_t *sketch = l2_sketch->sketch;
	uint32_t d = sketch_depth(sketch->sketch);

	uint32_t i;
	int64_t new;

	int64_t *sum = l2_sketch->sum;
	int64_t *old = l2_sketch->median;

	assert(value >= 0);

	for (i = 0; i < d; i++) {
		old[i] = sketch_point_partial(sketch, id, i);

		sum[i] -= (old[i] * old[i]);
		assert(sum[i] >= 0);
	}

	sketch_update(sketch, id, value);

	for (i = 0; i < d; i++) {
		new = old[i] + value;

		assert(new == sketch_point_partial(sketch, id, i));

		sum[i] += (new * new);
	}
}

int64_t l2_sketch_norm(l2_sketch_t *l2_sketch) {
	sketch_t *sketch = l2_sketch->sketch->sketch;
	uint32_t d = sketch_depth(sketch);

	int64_t *sum    = l2_sketch->sum;
	int64_t *median = l2_sketch->median;
	uint32_t i;

	for (i = 0; i < d; i++) {
		assert(sum[i] >= 0);

		median[i] = (int64_t) sqrt(sum[i]);
	}

	return median_wirth(median, d);
}

void l2_sketch_destroy(l2_sketch_t *l2_sketch) {
	if (l2_sketch == NULL) {
		return;
	}

	if (l2_sketch->sum != NULL) {
		free(l2_sketch->sum);
	}

	free(l2_sketch);
}
