#ifndef H_hash
#define H_hash

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "xutil.h"

//#define MOD_P    ((uint64_t)4294971673)
#define MOD_P    ((uint64_t)2305843009213693951)
#define BYTE     8
#define BIT32    31

typedef uint32_t(*hash)(uint32_t w, uint8_t M, uint32_t x, uint64_t a, 
		uint64_t b);
typedef uint64_t(*agen)();
typedef uint64_t(*bgen)(uint8_t M);

typedef struct {
	hash hash;
	agen agen;
	bgen bgen;
	uint8_t c;
} hash_t;

uint32_t ms(uint32_t w, uint8_t M, uint32_t x, uint64_t a, uint64_t b);
uint64_t ms_agen();
uint64_t ms_bgen(uint8_t M);

uint32_t ms2(uint32_t w, uint8_t M, uint32_t x, uint64_t a, uint64_t b);
uint64_t ms2_agen();
uint64_t ms2_bgen(uint8_t M);

uint32_t cw(uint32_t w, uint8_t M, uint32_t x, uint64_t a, uint64_t b);
uint32_t cwp2(uint32_t w, uint8_t M, uint32_t x, uint64_t a, uint64_t b);
uint64_t cw_agen();
uint64_t cw_bgen(uint8_t M);

uint32_t cw2(uint32_t w, uint8_t M, uint32_t x, uint64_t a, uint64_t b);
uint32_t cw2p2(uint32_t w, uint8_t M, uint32_t x, uint64_t a, uint64_t b);
uint64_t cw2_bgen(uint8_t M);

inline int8_t sign_cw(uint32_t x, uint64_t a, uint64_t b) {
	uint64_t res = a * (uint64_t)x + b;
	res = (res & MOD_P);
	assert( a > 0 );

	assert( a < MOD_P );

	assert( b < MOD_P );

	return (res & ((uint64_t)1 << BIT32)) ? 1 : -1;
}

inline uint64_t sign_cw_agen () {
	uint64_t a = 1 + xuni_rand()*(MOD_P-1);

	assert(a < MOD_P);
	assert(a > 0);

	return a;
}

inline uint64_t sign_cw_bgen () {
	uint64_t b = xuni_rand() * MOD_P;
	
	assert(b < MOD_P);
	
	return b;
}

inline int8_t sign_ms(uint32_t x, uint64_t a, uint64_t b) {
	// a < 2^w
	assert( a <= UINT32_MAX ) ;

	// a is odd
	assert( a&1 ) ;

	// b < 2^w-M
	assert( b < pow(2, sizeof(uint32_t)*BYTE-1) ) ;

	return ((uint32_t) (a*x+b) >> (sizeof(uint32_t)*BYTE-1)) ? 1 : -1;
}

inline uint64_t sign_ms_agen () {
	return (uint32_t)0x1 |(uint32_t)(xuni_rand() * UINT32_MAX);
}

inline uint64_t sign_ms_bgen () {
	return xuni_rand() * ((uint32_t)1 << (sizeof(uint32_t)*BYTE-1));
}

extern hash_t multiplyShift;
extern hash_t multiplyShift2;
extern hash_t carterWegman;
extern hash_t carterWegmanp2;
extern hash_t carterWegman2;
extern hash_t carterWegman2p2;

void hash_init(uint8_t *restrict M, uint32_t width);

#endif
