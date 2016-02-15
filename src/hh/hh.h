#ifndef H_hh
#define H_hh

#include <stdint.h>

// User defined libraries
#include "hash.h"

typedef struct {
	uint32_t *hitters;
	uint32_t count;
} heavy_hitter_t;

typedef void*(*hh_create)(void *params);
typedef void(*hh_destroy)(void *hh);
typedef void(*hh_update)(void *hh, uint32_t idx, int32_t c);
typedef heavy_hitter_t*(*hh_query)();

typedef struct {
	hh_create   create;
	hh_destroy  destroy;
	hh_update   update;
	hh_query    query;
} hh_func_t;

typedef struct {
	void      *hh;	
	hh_func_t *funcs;	
} hh_t;

typedef struct {
	//sketch_func_t *f;
	hash_t *hash;
	void * params;
} heavy_hitter_params_t;

hh_t *heavy_hitter_create(hh_func_t *f, void *params);

// Destuction
void heavy_hitter_destroy(hh_t *hh);

// Update
void heavy_hitter_update(hh_t *hh, uint32_t idx, int64_t c);

// Query
heavy_hitter_t *heavy_hitter_query(hh_t *hh);

extern hh_func_t hh_sketch;
extern hh_func_t hh_const_sketch;

#endif
