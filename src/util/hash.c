#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include <math.h>

#include "hash.h"
#include "xutil.h"

/*****************************************************************************
 *                              CARTER-WEGMAN                                *
 *****************************************************************************/

uint32_t cw(uint32_t w, uint8_t M, uint32_t x, uint64_t a, uint64_t b) {
	(void) M;
	
	uint64_t res = a * (uint64_t)x + b;
	res = (res & MOD_P);

	assert( a > 0 );

	assert( a < MOD_P );

	assert( b < MOD_P );

	assert( res < MOD_P );

	return (uint32_t)(res % (uint64_t)w);
}

uint32_t cwp2(uint32_t w, uint8_t M, uint32_t x, uint64_t a, uint64_t b) {
	(void) M;
	assert( a > 0 );

	assert( a < MOD_P );

	assert( b < MOD_P );

	// Amount of bins must be power of 2
	assert( w && !(w & (w - 1)) );

	return ((((a*x)+b) & MOD_P) & (w-1));
}

uint64_t cw_agen () {
	uint64_t a = 1 + (uint64_t)(xuni_rand()*(MOD_P-1));

	assert(a < MOD_P);
	assert(a > 0);

	return a;
}

uint64_t cw_bgen (uint8_t M) {
	(void) M;
	uint64_t b = (uint64_t)(xuni_rand() * MOD_P);
	
	assert(b < MOD_P);
	
	return b;
}

/*****************************************************************************
 *                              MULTIPLY-SHIFT                               *
 *****************************************************************************/
uint32_t ms(uint32_t w, uint8_t M, uint32_t x, uint64_t a, uint64_t b) {
	(void) w;

	// a < 2^w
	assert( a <= UINT32_MAX ) ;

	// a is odd
	assert( a&1 ) ;

	// b < 2^w-M
	assert( b < pow(2, sizeof(uint32_t)*BYTE-M) ) ;

	// Amount of bins must be power of 2
	assert( w && !(w & (w - 1)) );

	return (uint32_t) (a*x+b) >> (sizeof(uint32_t)*BYTE-M);
}

uint64_t ms_agen () {
	return (uint32_t)0x1 |(uint32_t)(xuni_rand() * UINT32_MAX);
}

uint64_t ms_bgen (uint8_t M) {
	return xuni_rand() * (1 << (sizeof(uint32_t)*BYTE-M));
}

/*****************************************************************************
 *                              MULTIPLY-SHIFT-2                             *
 *****************************************************************************/

uint32_t ms2(uint32_t w, uint8_t M, uint32_t x, uint64_t a, uint64_t b) {
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

uint64_t ms2_agen () {
	return (uint32_t)0x1 |(uint32_t)(xuni_rand() * UINT32_MAX);
}

uint64_t ms2_bgen (uint8_t M) {
	(void) M;
	return 0;
}

/*****************************************************************************
 *                              CARTER-WEGMAN                                *
 *****************************************************************************/
uint32_t cw2(uint32_t w, uint8_t M, uint32_t x, uint64_t a, uint64_t b) {
	(void) b;
	(void) M;

	return ((a*x) & MOD_P) % w;
}

uint32_t cw2p2(uint32_t w, uint8_t M, uint32_t x, uint64_t a, uint64_t b) {
	(void) b;
	(void) M;

	// Amount of bins must be power of 2
	assert( w && !(w & (w - 1)) );

	return ((a*x) & MOD_P) & (w-1);
}

uint64_t cw2_bgen (uint8_t M) {
	(void) M;
	return 0;
}

/*****************************************************************************
 *                                    SIGN                                   *
 *****************************************************************************/

extern inline int8_t   sign_ms(uint32_t x, uint64_t a, uint64_t b);
extern inline uint64_t sign_ms_agen();
extern inline uint64_t sign_ms_bgen();

extern inline int8_t   sign_cw(uint32_t x, uint64_t a, uint64_t b);
extern inline uint64_t sign_cw_agen();
extern inline uint64_t sign_cw_bgen();

/*****************************************************************************
 *                           HASH_T STRUCTURES                               *
 *****************************************************************************/

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
	.c    = 1,
};

hash_t carterWegmanp2 = {
	.hash = (hash) cwp2,
	.agen = (agen) cw_agen,
	.bgen = (bgen) cw_bgen,
	.c    = 1,
};

hash_t carterWegman2 = {
	.hash = (hash) cw2,
	.agen = (agen) cw_agen,
	.bgen = (bgen) cw2_bgen,
	.c    = 2,
};

hash_t carterWegman2p2 = {
	.hash = (hash) cw2p2,
	.agen = (agen) cw_agen,
	.bgen = (bgen) cw2_bgen,
	.c    = 2,
};

void hash_init(uint8_t *restrict M, uint32_t width) {
	*M = (uint8_t)floor(log2(width));
}
