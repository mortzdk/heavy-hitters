#ifndef H_hash
#define H_hash

#include <stdint.h>
#include <assert.h>

#include "xutil.h"

#define MOD_SIGN 2
#define MOD_P    2147483647
#define BYTE     8

typedef uint32_t(*hash)(uint32_t w, uint8_t M, uint32_t x, uint32_t a, 
		uint32_t b);
typedef uint32_t(*agen)();
typedef uint32_t(*bgen)(uint8_t M);

typedef struct {
	hash hash;
	agen agen;
	bgen bgen;
	uint8_t c;
} hash_t;

uint32_t h31(uint32_t w, uint8_t M, uint32_t x, uint32_t a, uint32_t b);
uint32_t h31p2(uint32_t w, uint8_t M, uint32_t x, uint32_t a, uint32_t b);
uint32_t h31_agen();
uint32_t h31_bgen();

uint32_t ms(uint32_t w, uint8_t M, uint32_t x, uint32_t a, uint32_t b);
uint32_t ms_agen();
uint32_t ms_bgen(uint8_t M);

uint32_t ms2(uint32_t w, uint8_t M, uint32_t x, uint32_t a, uint32_t b);
uint32_t ms2_agen();
uint32_t ms2_bgen();

uint32_t cw(uint32_t w, uint8_t M, uint32_t x, uint32_t a, uint32_t b);
uint32_t cw_agen();
uint32_t cw_bgen();

inline int8_t sign(uint32_t x, uint8_t a, uint8_t b) {
	return (((a*x)+b) & MOD_SIGN) ? 1 : -1;
}

inline uint8_t sign_agen () {
	uint8_t a = 1 + xuni_rand()*2;

	assert(a < MOD_SIGN+1);

	return a;
}

inline uint8_t sign_bgen () {
	uint8_t b = xuni_rand() * (MOD_SIGN+1);
	
	assert(b < MOD_SIGN+1);
	
	return b;
}

extern hash_t hash31;
extern hash_t hash31p2;
extern hash_t multiplyShift;
extern hash_t multiplyShift2;
extern hash_t carterWegman;

void hash_init(uint8_t *restrict M, uint32_t width);

#endif
