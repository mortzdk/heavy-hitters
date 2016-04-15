#include <criterion/criterion.h>
#include <math.h>
#include <string.h>

#include "util/hash.h"
#include "util/xutil.h"

#define EPSILON  (0.05)
#define UNI_RUNS (1000000)
#define GEN_RUNS (1000)
#define IMPLS    (6)
#define CW_IMPLS (4)
#define MS_IMPLS (2)

hash_t *hashes[IMPLS] = {
	&carterWegman,
	&carterWegmanp2,
	&carterWegman2,
	&carterWegman2p2,
	&multiplyShift,
	&multiplyShift2
};

Test(hash, cw_a_gen, .disabled=0) {
	uint64_t a;
	for (int j = 0; j < GEN_RUNS; j++) {
		for (int i = 0; i < CW_IMPLS; i++) {
			a = hashes[i]->agen();

			cr_assert( a > 0 );
			cr_assert( a < MOD_P );
		}
	}
}

Test(hash, cw_b_gen, .disabled=0) {
	uint64_t b;
	for (int j = 0; j < GEN_RUNS; j++) {
		for (int i = 0; i < CW_IMPLS; i++) {
			b = hashes[i]->bgen(0);

			cr_assert( b < MOD_P );
		}
	}
}

Test(hash, ms_a_gen, .disabled=0) {
	uint64_t a;
	for (int j = 0; j < GEN_RUNS; j++) {
		for (int i = CW_IMPLS; i < IMPLS; i++) {
			a = hashes[i]->agen();

			// a < 2^w
			cr_assert( a <= UINT32_MAX ) ;

			// a is odd
			cr_assert( a&1 ) ;
		}
	}
}

Test(hash, ms_b_gen, .disabled=0) {
	uint64_t b;
	for (int j = 0; j < GEN_RUNS; j++) {
		for (int i = CW_IMPLS; i < IMPLS; i++) {
			b = hashes[i]->bgen(8);

			// b < 2^w-M
			cr_assert( b < pow(2, sizeof(uint32_t)*BYTE-8) ) ;
		}
	}
}

Test(hash, cw_uniform, .disabled=0) {
	uint64_t a, b;
	const uint32_t m        = UINT32_MAX;
	const uint32_t w        = 100;
	const uint32_t M        = floor(log2(w));
	uint64_t c[w];

	memset(c, '\0', sizeof(uint64_t)*w);

	a = hashes[0]->agen();
	b = hashes[0]->bgen(0);
	for (uint32_t i = 0; i < UNI_RUNS; i++) {
		c[hashes[0]->hash(w, M, (uint32_t)(xuni_rand()*m), a, b)] += 1;
	}

	for (uint32_t i = 0; i < w; i++) {
		uint32_t exp = (uint32_t)UNI_RUNS/w;
		double err = (double)(abs((int32_t)exp-(int32_t)c[i]))/exp;
		cr_assert( err < EPSILON );
	}
}

Test(hash, cwp2_uniform, .disabled=0) {
	uint64_t a, b;
	const uint32_t m        = UINT32_MAX;
	const uint32_t w        = 128;
	const uint32_t M        = floor(log2(w));
	uint64_t c[w];

	memset(c, '\0', sizeof(uint64_t)*w);

	a = hashes[1]->agen();
	b = hashes[1]->bgen(0);
	for (uint32_t i = 0; i < UNI_RUNS; i++) {
		c[hashes[1]->hash(w, M, (uint32_t)(xuni_rand()*m), a, b)] += 1;
	}

	for (uint32_t i = 0; i < w; i++) {
		uint32_t exp = (uint32_t)UNI_RUNS/w;
		double err = (double)(abs((int32_t)exp-(int32_t)c[i]))/exp;
		cr_assert( err < EPSILON );
	}
}

Test(hash, cw2_uniform, .disabled=0) {
	uint64_t a, b;
	const uint32_t m        = UINT32_MAX;
	const uint32_t w        = 100;
	const uint32_t M        = floor(log2(w));
	uint64_t c[w];

	memset(c, '\0', sizeof(uint64_t)*w);

	a = hashes[2]->agen();
	b = hashes[2]->bgen(0);
	for (uint32_t i = 0; i < UNI_RUNS; i++) {
		c[hashes[2]->hash(w, M, (uint32_t)(xuni_rand()*m), a, b)] += 1;
	}

	for (uint32_t i = 0; i < w; i++) {
		uint32_t exp = (uint32_t)UNI_RUNS/w;
		double err = (double)(abs((int32_t)exp-(int32_t)c[i]))/exp;
		cr_assert( err < EPSILON );
	}
}

Test(hash, cw2p2_uniform, .disabled=0) {
	uint64_t a, b;
	const uint32_t m        = UINT32_MAX;
	const uint32_t w        = 128;
	const uint32_t M        = floor(log2(w));
	uint64_t c[w];

	memset(c, '\0', sizeof(uint64_t)*w);

	a = hashes[3]->agen();
	b = hashes[3]->bgen(0);
	for (uint32_t i = 0; i < UNI_RUNS; i++) {
		c[hashes[3]->hash(w, M, (uint32_t)(xuni_rand()*m), a, b)] += 1;
	}

	for (uint32_t i = 0; i < w; i++) {
		uint32_t exp = (uint32_t)UNI_RUNS/w;
		double err = (double)(abs((int32_t)exp-(int32_t)c[i]))/exp;
		cr_assert( err < EPSILON );
	}
}

Test(hash, ms_uniform, .disabled=0) {
	uint64_t a, b;
	const uint32_t m        = UINT32_MAX;
	const uint32_t w        = 128;
	const uint32_t M        = floor(log2(w));
	uint64_t c[w];

	memset(c, '\0', sizeof(uint64_t)*w);

	a = hashes[4]->agen();
	b = hashes[4]->bgen(0);
	for (uint32_t i = 0; i < UNI_RUNS; i++) {
		c[hashes[4]->hash(w, M, (uint32_t)(xuni_rand()*m), a, b)] += 1;
	}

	for (uint32_t i = 0; i < w; i++) {
		uint32_t exp = (uint32_t)UNI_RUNS/w;
		double err = (double)(abs((int32_t)exp-(int32_t)c[i]))/exp;
		cr_assert( err < EPSILON );
	}
}

Test(hash, ms2_uniform, .disabled=0) {
	uint64_t a, b;
	const uint32_t m        = UINT32_MAX;
	const uint32_t w        = 128;
	const uint32_t M        = floor(log2(w));
	uint64_t c[w];

	memset(c, '\0', sizeof(uint64_t)*w);

	a = hashes[5]->agen();
	b = hashes[5]->bgen(0);
	for (uint32_t i = 0; i < UNI_RUNS; i++) {
		c[hashes[5]->hash(w, M, (uint32_t)(xuni_rand()*m), a, b)] += 1;
	}

	for (uint32_t i = 0; i < w; i++) {
		uint32_t exp = (uint32_t)UNI_RUNS/w;
		double err = (double)(abs((int32_t)exp-(int32_t)c[i]))/exp;
		cr_assert( err < EPSILON );
	}
}

/*****************************************************************************
 *                                SIGN                                       *
 *****************************************************************************/

Test(hash, sign_cw_a_gen, .disabled=0) {
	uint64_t a;
	for (int j = 0; j < GEN_RUNS; j++) {
		a = sign_cw_agen();

		cr_assert(a < MOD_P);
		cr_assert(a > 0);
	}
}

Test(hash, sign_cw_b_gen, .disabled=0) {
	uint64_t b;
	for (int j = 0; j < GEN_RUNS; j++) {
		b = sign_cw_bgen();

		// b < 2^w-M
		cr_assert(b < MOD_P);
	}
}

Test(hash, sign_cw_uniform, .disabled=0) {
	uint64_t a, b;
	const uint32_t m        = UINT32_MAX;
	const uint32_t w        = 2;
	uint64_t c[w];

	memset(c, '\0', sizeof(uint64_t)*w);

	a = sign_cw_agen();
	b = sign_cw_bgen();
	for (uint32_t i = 0; i < UNI_RUNS; i++) {
		c[sign_cw((uint32_t)(xuni_rand()*m), a, b) == -1] += 1;
	}

	for (uint32_t i = 0; i < w; i++) {
		uint32_t exp = (uint32_t)UNI_RUNS/w;
		double err = (double)(abs((int32_t)exp-(int32_t)c[i]))/exp;
		cr_assert( err < EPSILON );
	}
}

Test(hash, sing_cw_pairwise, .disabled=0) {
	uint64_t a, b;
	const uint32_t m        = UINT32_MAX;
	const uint32_t w        = 2;
	uint64_t c[w][w];

	memset(c, '\0', sizeof(uint64_t)*w*w);

	a = sign_cw_agen();
	b = sign_cw_bgen();
	for (uint32_t i = 0; i < UNI_RUNS; i++) {

		c[sign_cw((uint32_t)(xuni_rand()*m), a, b) == -1]
		 [sign_cw((uint32_t)(xuni_rand()*m), a, b) == -1] += 1;
	}

	for (uint32_t i = 0; i < w; i++) {
		for (uint32_t j = 0; j < w; j++) {
			uint32_t exp = (uint32_t)UNI_RUNS/(w*w);
			double err   = (double)(abs((int32_t)exp-(int32_t)c[i][j]))/exp;
			cr_assert( err < EPSILON );
		}
	}
}

Test(hash, sign_ms_a_gen, .disabled=0) {
	uint64_t a;
	for (int j = 0; j < GEN_RUNS; j++) {
		a = sign_ms_agen();

		// a < 2^w
		cr_assert( a <= UINT32_MAX ) ;

		// a is odd
		cr_assert( a&1 ) ;
	}
}

Test(hash, sign_ms_b_gen, .disabled=0) {
	uint64_t b;
	for (int j = 0; j < GEN_RUNS; j++) {
		b = sign_ms_bgen();

		// b < 2^w-M
		cr_assert( b < pow(2, sizeof(uint32_t)*BYTE-1) ) ;
	}
}

Test(hash, sign_ms_uniform, .disabled=0) {
	uint64_t a, b;
	const uint32_t m        = UINT32_MAX;
	const uint32_t w        = 2;
	uint64_t c[w];

	memset(c, '\0', sizeof(uint64_t)*w);

	a = sign_ms_agen();
	b = sign_ms_bgen();
	for (uint32_t i = 0; i < UNI_RUNS; i++) {
		c[sign_ms((uint32_t)(xuni_rand()*m), a, b) == -1] += 1;
	}

	for (uint32_t i = 0; i < w; i++) {
		uint32_t exp = (uint32_t)UNI_RUNS/w;
		double err = (double)(abs((int32_t)exp-(int32_t)c[i]))/exp;
		cr_assert( err < EPSILON );
	}
}

Test(hash, sing_ms_pairwise, .disabled=0) {
	uint64_t a, b;
	const uint32_t m        = UINT32_MAX;
	const uint32_t w        = 2;
	uint64_t c[w][w];

	memset(c, '\0', sizeof(uint64_t)*w*w);

	a = sign_ms_agen();
	b = sign_ms_bgen();
	for (uint32_t i = 0; i < UNI_RUNS; i++) {

		c[sign_ms((uint32_t)(xuni_rand()*m), a, b) == -1]
		 [sign_ms((uint32_t)(xuni_rand()*m), a, b) == -1] += 1;
	}

	for (uint32_t i = 0; i < w; i++) {
		for (uint32_t j = 0; j < w; j++) {
			uint32_t exp = (uint32_t)UNI_RUNS/(w*w);
			double err   = (double)(abs((int32_t)exp-(int32_t)c[i][j]))/exp;
			cr_assert( err < EPSILON );
		}
	}
}
