// Standard libraries
#include <stdint.h>

// User defined libraries
#include "xutil.h"

#include "hh/hh.h"
#include "hh/const_sketch.h"
#include "hh/sketch.h"

hh_func_t hh_sketch = {
	.create   = (hh_create)  hh_sketch_create,
	.destroy  = (hh_destroy) hh_sketch_destroy,
	.update   = (hh_update)  hh_sketch_update,
	.query    = (hh_query)   hh_sketch_query,
};

hh_func_t hh_const_sketch = {
	.create   = (hh_create)  hh_const_sketch_create,
	.destroy  = (hh_destroy) hh_const_sketch_destroy,
	.update   = (hh_update)  hh_const_sketch_update,
	.query    = (hh_query)   hh_const_sketch_query,
};

hh_t *heavy_hitter_create(hh_func_t *restrict f, void *restrict params) {
	hh_t *hh   = xmalloc( sizeof(hh_t) ); 

	hh->funcs  = f;
	hh->hh     = f->create(params);

	return hh;
}

void heavy_hitter_destroy(hh_t *restrict hh) {
	if (hh == NULL) {
		return;
	}

	hh->funcs->destroy(hh->hh);

	free(hh);
	hh = NULL;
}

void heavy_hitter_update(hh_t *restrict hh, const uint32_t i, const int64_t c) {
	hh->funcs->update(hh->hh, i, c);
}

heavy_hitter_t *heavy_hitter_query(hh_t *restrict hh) {
	return hh->funcs->query(hh->hh);
}

