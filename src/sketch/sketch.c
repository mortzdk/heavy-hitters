// Standard libraries
#include <stdint.h>

// User defined libraries
#include "xutil.h"

#include "sketch/count_min.h"
#include "sketch/sketch.h"

extern inline uint32_t sketch_depth(void *sketch);
extern inline uint32_t sketch_width(void *sketch);

sketch_func_t countMin = {
	.create   = (s_create)  count_min_create,
	.destroy  = (s_destroy) count_min_destroy,
	.update   = (s_update)  count_min_update,
	.point    = (s_point)   count_min_point,
	.above    = (s_above)   count_min_above_thresshold,
	.rangesum = (s_rangesum)count_min_range_sum,
};

sketch_t *sketch_create(sketch_func_t *f, hash_t *hash, uint8_t b, 
		double epsilon, double delta) {
	sketch_t *s = xmalloc( sizeof(sketch_t) ); 
	s->funcs    = f;
	s->sketch   = f->create(hash, b, epsilon, delta);

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

void sketch_update(sketch_t *s, uint32_t i, int64_t c) {
	s->funcs->update(s->sketch, i, c);
}

uint64_t sketch_point(sketch_t *s, uint32_t i) {
	return s->funcs->point(s->sketch, i);
}

bool sketch_above_thresshold(sketch_t *s, uint32_t i, uint64_t th) {
	return s->funcs->above(s->sketch, i, th);
}

uint64_t sketch_range_sum(sketch_t *s, uint32_t l, uint32_t r) {
	return s->funcs->rangesum(s->sketch, l , r);
}
