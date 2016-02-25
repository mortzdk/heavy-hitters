#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include <math.h>

#include "xutil.h"
#include "hash.h"

/*****************************************************************************
 *        HASH_31 by MASSDAL: http://www.cs.rutgers.edu/~muthu/prng.c        *
 *****************************************************************************/

static inline uint32_t h31_internal(uint32_t x, uint32_t a, uint32_t b) {
	uint64_t result;

	result   = (a * x) + b;
	result   = ( (result >> 31) + result ) & MOD_P;

	return (uint32_t) result;
}

uint32_t h31(uint32_t w, uint8_t M, uint32_t x, uint32_t a, uint32_t b) {
	uint32_t res = h31_internal(x, a, b);

	(void) M;

	return res % w;
}

uint32_t h31p2(uint32_t w, uint8_t M, uint32_t x, uint32_t a, uint32_t b) {
	uint32_t res = h31_internal(x, a, b);

	(void) M;

	// Amount of bins must be power of 2
	assert( w && !(w & (w - 1)) );

	return res & (w - 1);
	
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
uint32_t ms(uint32_t w, uint8_t M, uint32_t x, uint32_t a, uint32_t b) {
	(void) w;

	// a < 2^w
	assert( a <= UINT32_MAX ) ;

	// a is odd
	assert( a&1 ) ;

	// b < 2^w-M
	assert( b <= pow(2, sizeof(uint32_t)*BYTE-M) ) ;

	// Amount of bins must be power of 2
	assert( w && !(w & (w - 1)) );

	return (uint32_t) (a*x+b) >> (sizeof(uint32_t)*BYTE-M);
}

uint32_t ms_agen () {
	return (uint32_t)0x1 |(uint32_t)(xuni_rand() * UINT32_MAX);
}

uint32_t ms_bgen (uint8_t M) {
	return xuni_rand() * (1 << (sizeof(uint32_t)*BYTE-M));
}

/*****************************************************************************
 *                              MULTIPLY-SHIFT-2                             *
 *****************************************************************************/

uint32_t ms2(uint32_t w, uint8_t M, uint32_t x, uint32_t a, uint32_t b) {
	(void) b;
	(void) w;

	// a < 2^w
	assert( a <= UINT32_MAX ) ;

	// a is odd
	assert( a&1 ) ;

	// Amount of bins must be power of 2
	assert( w && !(w & (w - 1)) );

	return (uint32_t) (a*x) >> (sizeof(uint32_t)*BYTE-M);
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
uint32_t cw(uint32_t w, uint8_t M, uint32_t x, uint32_t a, uint32_t b) {
	(void) b;
	(void) M;

	// Amount of bins must be power of 2
	assert( w && !(w & (w - 1)) );

	return (((a*x) >> 31) & MOD_P) & w;
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

extern inline int8_t  sign(uint32_t x, uint8_t a, uint8_t b);
extern inline uint8_t sign_agen();
extern inline uint8_t sign_bgen();

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

void hash_init(uint8_t *restrict M, uint32_t width) {
	*M = (uint8_t)floor(log2(width));
}
