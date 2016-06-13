// Standard libraries
#include <stdint.h>
#include <stdio.h>

// User defined libraries
#include "util/xutil.h"

#include "hh/hh.h"
#include "hh/hh_measure.h"

hh_measure_t *heavy_hitter_measure_create(heavy_hitter_params_t *restrict params) {
	hh_measure_t *hh = xmalloc( sizeof(hh_measure_t) ); 
	hh->funcs        = params->f;
	hh->hh           = hh->funcs->create(params);

	return hh;
}

void heavy_hitter_measure_destroy(hh_measure_t *restrict params) {
	if (params->hh == NULL) {
		return;
	}

	params->funcs->destroy(params->hh);

	free(params);
	params = NULL;
}

void heavy_hitter_measure_update(hh_measure_t *restrict params) {
	params->funcs->update(params->hh, params->i, params->c);
}

heavy_hitter_t *heavy_hitter_measure_query(hh_measure_t *restrict params) {
	return params->funcs->query(params->hh);
}

