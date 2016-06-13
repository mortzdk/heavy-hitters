// Probabilistic Random Number Generators
// Collected from various sources by Graham Cormode, 2000-2003
// 
#ifndef _PRNG_h
#define _PRNG_h

#define _USE_MATH_DEFINES

#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#ifdef _MSC_VER
typedef unsigned __int8 byte;
//typedef __int8 int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#endif

#define MOD 2147483647
#define HL 31

extern long comode_hash31(int64_t, int64_t, int64_t);
extern long fourwise(int64_t, int64_t, int64_t, int64_t, int64_t);

#define KK  17
#define NTAB 32

#define SWAP(a,b) temp=(a);(a)=(b);(b)=temp;

int64_t LLMedSelect(int k, int n, int64_t arr[]);
int MedSelect(int k, int n, int arr[]);

typedef struct prng_type{
  int usenric; // which prng to use
  float scale;             /* 2^(- integer size) */
  long floatidum;
  long intidum; // needed to keep track of where we are in the 
  // nric random number generators
  long iy;
  long iv[NTAB];
  /* global variables */
  unsigned long randbuffer[KK];  /* history buffer */
  int r_p1, r_p2;          /* indexes into history buffer */
  int iset;
  double gset;
} prng_type;

extern long prng_int(prng_type *);
extern float prng_float(prng_type *);
extern prng_type * prng_Init(long, int);
extern void prng_Destroy(prng_type * prng);
void prng_Reseed(prng_type *, long);

//extern long double zipf(double, long) ;
extern double fastzipf(double, long, double, prng_type *);
extern double zeta(long, double);
extern double prng_normal(prng_type * prng);
extern double prng_stable(prng_type * prng, double);

#endif
