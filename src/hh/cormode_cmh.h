#ifndef COUNTMIN_IMPL_h
#define COUNTMIN_IMPL_h

#include "hh/hh.h"

typedef struct {
	double          phi;
	double          epsilon;
	double          delta;
	uint32_t        m;
	uint32_t        b;
} hh_cormode_cmh_params_t;

///////////////////////////////////////////////////////////////////////////////
#define min(x,y)	((x) < (y) ? (x) : (y))
#define max(x,y)	((x) > (y) ? (x) : (y))

typedef struct CMH_type{
  int64_t count;
  int U; // size of the universe in bits
  int gran; // granularity: eg 1, 4 or 8 bits
  int levels; // function of U and gran
  int freelim; // up to which level to keep exact counts
  int depth;
  int width;
  int ** counts;
  unsigned int **hasha, **hashb;
  int L1;
  double phi;
} CMH_type;
///////////////////////////////////////////////////////////////////////////////

// Initialization
CMH_type *hh_cormode_cmh_create(heavy_hitter_params_t *restrict p);

// Destruction
void hh_cormode_cmh_destroy(CMH_type *restrict hh);

// Update
void hh_cormode_cmh_update(CMH_type *restrict hh, const uint32_t idx, 
		const int64_t c);

// Query
heavy_hitter_t *hh_cormode_cmh_query(CMH_type *restrict hh);

#endif
