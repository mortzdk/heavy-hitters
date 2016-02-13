// Standard libraries
#include <stdint.h>

// User defined libraries
#include "xutil.h"

#include "hh/hh.h"
#include "hh/sketch.h"

hh_func_t hh_sketch = {
	.create   = (hh_create)  hh_sketch_create,
	.destroy  = (hh_destroy) hh_sketch_destroy,
	.update   = (hh_update)  hh_sketch_update,
	.query    = (hh_query)   hh_sketch_query,
};

hh_t *heavy_hitter_create(hh_func_t *f, void *params) {
	hh_t *hh   = xmalloc( sizeof(hh_t) ); 

	hh->funcs  = f;
	hh->hh     = f->create(params);

	return hh;
}

void heavy_hitter_destroy(hh_t *hh) {
	if (hh == NULL) {
		return;
	}

	hh->funcs->destroy(hh->hh);

	free(hh);
	hh = NULL;
}

void heavy_hitter_update(hh_t *hh, uint32_t i, int64_t c) {
	hh->funcs->update(hh->hh, i, c);
}

heavy_hitter_t *heavy_hitter_query(hh_t *hh) {
	return hh->funcs->query(hh->hh);
}

