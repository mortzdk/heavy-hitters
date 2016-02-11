// Standard libraries
#include <inttypes.h>

// User defined libraries
#include "xutil.h"

#include "sketch/count_min.h"
#include "sketch/sketch.h"

sketch_func_t countMin = {
	.create   = (create)  count_min_create,
	.destroy  = (destroy) count_min_destroy,
	.update   = (update)  count_min_update,
	.point    = (point)   count_min_point,
	.rangesum = (rangesum)count_min_range_sum,
};

sketch_t *sketch_create(sketch_func_t *f, hash_t *hash, short b, 
		double epsilon, double delta) {
	sketch_t *s = xmalloc( sizeof(sketch_t) ); 
	s->funcs  = f;
	s->sketch = s->funcs->create(b, epsilon, delta, hash);

	return s;
}

void sketch_destroy(sketch_t *s) {
	if (s == NULL) {
		return;
	}

	s->funcs->destroy(s->sketch);

	free(s);
	s = NULL;
}

void sketch_update(sketch_t *s, uint32_t i, int32_t c) {
	s->funcs->update(s->sketch, i, c);
}

uint32_t sketch_point(sketch_t *s, uint32_t i) {
	return s->funcs->point(s->sketch, i);
}

uint32_t sketch_range_sum(sketch_t *s, uint32_t l, uint32_t r) {
	return s->funcs->rangesum(s->sketch, l , r);
}
