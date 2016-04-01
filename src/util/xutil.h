#ifndef H_XUTIL
#define H_XUTIL

#ifndef BYTE
#define BYTE 8
#endif

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

#include <stdlib.h>  /* random */
#include <inttypes.h>  /* random */

extern uint32_t I1, I2;

extern const uint8_t MultiplyDeBruijnBitPosition2[32];

inline uint32_t next_pow_2(register uint32_t v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

inline double xuni_rand(void) {
	I1 = 36969*(I1 & 0177777) + (I1>>16);
	I2 = 18000*(I2 & 0177777) + (I2>>16);
	return ((I1 << 16)^(I2 & 0177777)) * 2.328306437080797e-10; /* in [0,1) */
}

inline uint8_t xceil_log2(uint64_t x) {
    static const uint64_t t[6] = {
        0xFFFFFFFF00000000ull,
        0x00000000FFFF0000ull,
        0x000000000000FF00ull,
        0x00000000000000F0ull,
        0x000000000000000Cull,
        0x0000000000000002ull
    };

    register uint8_t y = (((x & (x - 1)) == 0) ? 0 : 1);
    register uint8_t j = 32;
    register uint8_t i;
	register uint8_t k;
    for (i = 0; i < 6; i++) {
        k = (((x & t[i]) == 0) ? 0 : j);
        y += k;
        x >>= k;
        j >>= 1;
    }

    return y;
}

void xerror(char *msg, int line, char *file);

void *xcalloc(size_t nmemb, size_t size);

void *xmalloc(size_t size);

void *xrealloc(void *ptr, size_t size);

#endif
