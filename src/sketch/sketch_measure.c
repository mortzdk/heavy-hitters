// Standard libraries
#include <stdint.h>

// User defined libraries
#include "sketch/sketch.h"
#include "sketch/sketch_measure.h"

#include "util/xutil.h"

sketch_measure_t *sketch_measure_create(sketch_measure_create_t *params) {
	sketch_measure_t *restrict s = xmalloc( sizeof(sketch_measure_t) ); 
	s->funcs  = params->f;
	s->sketch = s->funcs->create(params->hash, params->b, 
			params->epsilon, params->delta);
	return s;
}

void sketch_measure_destroy(sketch_measure_t *sm) {
	if ( unlikely(sm == NULL) ) {
		return;
	}

	sm->funcs->destroy(sm->sketch);

	free(sm);
	sm = NULL;
}

void sketch_measure_update(sketch_measure_t *sm) {
	sm->funcs->update(sm->sketch, sm->params.i, sm->params.c);
}

int64_t sketch_measure_point(sketch_measure_t *sm) {
	return sm->funcs->point(sm->sketch, sm->params.i);
}
