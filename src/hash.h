#ifndef H_hash
#define H_hash

#include <stdint.h>

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
uint32_t h31_agen ();
uint32_t h31_bgen ();

uint32_t ms(uint32_t w, uint8_t M, uint32_t x, uint32_t a, uint32_t b);
uint32_t ms_agen ();
uint32_t ms_bgen (uint8_t M);

extern hash_t hash31;
extern hash_t hash31p2;
extern hash_t multiplyShift;
extern hash_t multiplyShift2;
extern hash_t carterWegman;
extern hash_t sign;

void hash_init(uint8_t *restrict M, uint32_t width);

#endif
