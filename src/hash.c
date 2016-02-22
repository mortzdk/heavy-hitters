#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include <math.h>

#include "xutil.h"
#include "hash.h"

static uint32_t *restrict w;

/*****************************************************************************
 *        HASH_31 by MASSDAL: http://www.cs.rutgers.edu/~muthu/prng.c        *
 *****************************************************************************/

static inline uint32_t h31_internal(uint32_t x, uint32_t a, uint32_t b) {
	uint64_t result;

	result   = (a * x) + b;
	result   = ( (result >> 31) + result ) & MOD_P;

	return (uint32_t) result;
}

uint32_t h31(uint32_t x, uint32_t a, uint32_t b) {
	uint32_t res = h31_internal(x, a, b);

	return res % *w;
}

uint32_t h31p2(uint32_t x, uint32_t a, uint32_t b) {
	uint32_t res = h31_internal(x, a, b);

	// Amount of bins must be power of 2
	assert( w && !(w & (w - 1)) );

	return res & (*w - 1);
	
}

uint32_t h31_agen () {
	return 1 + (xuni_rand() * (MOD_P - 1));
}

uint32_t h31_bgen () {
	return xuni_rand() * (MOD_P - 1);
}

/*****************************************************************************
 *                              MULTIPLY-SHIFT                               *
 *****************************************************************************/
static uint8_t *restrict M;

uint32_t ms(uint32_t x, uint32_t a, uint32_t b) {
	// a < 2^w
	assert( a <= UINT32_MAX ) ;

	// a is odd
	assert( a&1 ) ;

	// b < 2^w-M
	assert( b <= pow(2, sizeof(uint32_t)*BYTE-M) ) ;

	// Amount of bins must be power of 2
	assert( w && !(w & (w - 1)) );

	return (uint32_t) (a*x+b) >> (sizeof(uint32_t)*BYTE-*M);
}

uint32_t ms_agen () {
	return (uint32_t)0x1 |(uint32_t)(xuni_rand() * UINT32_MAX);
}

uint32_t ms_bgen () {
	return xuni_rand() * (1 << (sizeof(uint32_t)*BYTE-*M));
}

/*****************************************************************************
 *                              MULTIPLY-SHIFT-2                             *
 *****************************************************************************/

uint32_t ms2(uint32_t x, uint32_t a, uint32_t b) {
	(void) b;

	// a < 2^w
	assert( a <= UINT32_MAX ) ;

	// a is odd
	assert( a&1 ) ;

	// Amount of bins must be power of 2
	assert( w && !(w & (w - 1)) );

	return (uint32_t) (a*x) >> (sizeof(uint32_t)*BYTE-*M);
}

uint32_t ms2_agen () {
	return (uint32_t)0x1 |(uint32_t)(xuni_rand() * UINT32_MAX);
}

uint32_t ms2_bgen () {
	return 0;
}

/*****************************************************************************
 *                              CARTER-WEGMAN                                *
 *****************************************************************************/
uint32_t cw(uint32_t x, uint32_t a, uint32_t b) {
	(void) b;

	// Amount of bins must be power of 2
	assert( w && !(w & (w - 1)) );

	return (((a*x) >> 31) & MOD_P) & *w;
}

uint32_t cw_agen () {
	return 1 + (xuni_rand() * (MOD_P - 1));
}

uint32_t cw_bgen () {
	return 0;
}

/*****************************************************************************
 *                                    SIGN                                   *
 *****************************************************************************/

uint8_t s(uint32_t x, uint8_t a, uint8_t b) {
	return (((a*x)+b) & MOD_SIGN);
}

uint8_t s_agen () {
	return 1 + (xuni_rand() >= 0.5);
}

uint8_t s_bgen () {
	double rand = xuni_rand();
	return (rand >= 0.66) + (rand >= 0.33);
}

/*****************************************************************************
 *                           HASH_T STRUCTURES                               *
 *****************************************************************************/

hash_t hash31 = {
	.hash = (hash) h31,
	.agen = (agen) h31_agen,
	.bgen = (bgen) h31_bgen,
	.c    = 1,
};

hash_t hash31p2 = {
	.hash = (hash) h31p2,
	.agen = (agen) h31_agen,
	.bgen = (bgen) h31_bgen,
	.c    = 1,
};

hash_t multiplyShift = {
	.hash = (hash) ms,
	.agen = (agen) ms_agen,
	.bgen = (bgen) ms_bgen,
	.c    = 1,
};

hash_t multiplyShift2 = {
	.hash = (hash) ms2,
	.agen = (agen) ms2_agen,
	.bgen = (bgen) ms2_bgen,
	.c    = 2,
};

hash_t carterWegman = {
	.hash = (hash) cw,
	.agen = (agen) cw_agen,
	.bgen = (bgen) cw_bgen,
	.c    = 2,
};

hash_t sign = {
	.hash = (hash) s,
	.agen = (agen) s_agen,
	.bgen = (bgen) s_bgen,
	.c    = 1,
};

void hash_width(hash_t *hash, uint32_t width) {
	if ( unlikely(!hash->w[0]) ) {
		w = &hash->w[0];
		M = &hash->M[0];
		*M = (uint8_t)floor(log2(width));
		*w = width;
	} else if ( unlikely(!hash->w[1]) ) {
		w = &hash->w[1];
		M = &hash->M[1];
		*M = (uint8_t)floor(log2(width));
		*w = width;
	} else if ( hash->w[0] == width ) {
		w = &hash->w[0];
		M = &hash->M[0];
	} else {
		w = &hash->w[1];
		M = &hash->M[1];
	} 
}
