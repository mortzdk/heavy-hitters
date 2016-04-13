#ifndef H_hh
#define H_hh

#include <stdint.h>

// User defined libraries
#include "util/hash.h"

typedef struct {
	uint32_t *restrict hitters;
	uint32_t count;
	uint32_t size;
} heavy_hitter_t;

typedef void*(*hh_create)(void *restrict params);
typedef void(*hh_destroy)(void *restrict hh);
typedef void(*hh_update)(void *restrict hh, const uint32_t idx, const int64_t c);
typedef heavy_hitter_t*(*hh_query)();

typedef struct {
	hh_create   create;
	hh_destroy  destroy;
	hh_update   update;
	hh_query    query;
} hh_func_t;

typedef struct {
	void      *restrict hh;	
	hh_func_t *restrict funcs;	
} hh_t;

typedef struct {
	hash_t    *restrict hash;
	void      *restrict params;
	hh_func_t *restrict f;
} heavy_hitter_params_t;

hh_t *heavy_hitter_create(heavy_hitter_params_t *restrict params);

// Destuction
void heavy_hitter_destroy(hh_t *restrict hh);

// Update
void heavy_hitter_update(hh_t *restrict hh, const uint32_t idx, const int64_t c);

// Query
heavy_hitter_t *heavy_hitter_query(hh_t *restrict hh);

extern hh_func_t hh_sketch;
extern hh_func_t hh_const_sketch;
extern hh_func_t hh_cormode_cmh;

#endif
