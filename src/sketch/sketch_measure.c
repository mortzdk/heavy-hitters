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

void sketch_measure_destroy(sketch_measure_params_t *params) {
	if ( unlikely(params->s == NULL) ) {
		return;
	}

	params->s->funcs->destroy(params->s->sketch);

	free(params->s);
	params->s = NULL;
}

void sketch_measure_update(sketch_measure_params_t *params) {
	params->s->funcs->update(params->s->sketch, params->i, params->c);
}

int64_t sketch_measure_point(sketch_measure_params_t *params) {
	return params->s->funcs->point(params->s->sketch, params->i);
}
