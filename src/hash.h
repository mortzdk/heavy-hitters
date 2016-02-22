#ifndef H_hash
#define H_hash

#include <stdint.h>

#define MOD_SIGN 2
#define MOD_P    2147483647
#define BYTE     8

typedef uint32_t(*hash)(uint32_t x, uint32_t a, uint32_t b);
typedef uint32_t(*agen)();
typedef uint32_t(*bgen)();

typedef struct {
	hash hash;
	agen agen;
	bgen bgen;
	uint8_t c;
	uint32_t w[2];
	uint8_t M[2];
} hash_t;

uint32_t h31p2(uint32_t x, uint32_t a, uint32_t b);
uint32_t h31_agen();
uint32_t h31_bgen();

uint32_t ms(uint32_t x, uint32_t a, uint32_t b);
uint32_t ms_agen();
uint32_t ms_bgen();

extern hash_t hash31;
extern hash_t hash31p2;
extern hash_t multiplyShift;

void hash_width(hash_t *hash, uint32_t width);

#endif
