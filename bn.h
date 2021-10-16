#ifndef BIGNUM__H
#define BIGNUM__H
/*

Big number library - arithmetic on multiple-precision unsigned integers.

This library is an implementation of arithmetic on arbitrarily large integers.

The difference between this and other implementations, is that the data structure
has optimal memory utilization (i.e. a 1024 bit integer takes up 128 bytes RAM),
and all memory is allocated statically: no dynamic allocation for better or worse.

Primary goals are correctness, clarity of code and clean, portable implementation.
Secondary goal is a memory footprint small enough to make it suitable for use in
embedded applications.


The current state is correct functionality and adequate performance.
There may well be room for performance-optimizations and improvements.

*/

#include <stdint.h>

#if !defined(bn_assert)
  #include <assert.h>
  #define bn_assert(e,s) assert((e)&&(s))
#endif

/* This macro defines the word size in bytes of the array that constitues the big-number data structure. */
#ifndef BN_WORD_SIZE
  #define BN_WORD_SIZE 4
#endif

/* Size of big-numbers in bytes */
#ifndef BN_ARRAY_SIZE
  #define BN_ARRAY_SIZE (128 / BN_WORD_SIZE)
#endif

#if (BN_WORD_SIZE == 1)
  typedef uint8_t  bn_word;
  typedef uint16_t bn_dword;
  #define bn_word_msb                 ((bn_dword)(0x80))
  #define BN_PRIword                  "%.02x"
  #define BN_SCAword                  "%2hhx"
  #define BN_MAX_VAL                  ((bn_dword)0xFF)
#elif (BN_WORD_SIZE == 2)
  typedef uint16_t bn_word;
  typedef uint32_t bn_dword;
  #define bn_word_msb                 ((bn_dword)(0x8000))
  #define BN_PRIword                  "%.04x"
  #define BN_SCAword                  "%4hx"
  #define BN_MAX_VAL                  ((bn_dword)0xFFFF)
#elif (BN_WORD_SIZE == 4)
  typedef uint32_t bn_word;
  typedef uint64_t bn_dword;
  #define bn_word_msb                 ((bn_dword)(0x80000000))
  #define BN_PRIword                  "%.08x"
  #define BN_SCAword                  "%8x"
  #define BN_MAX_VAL                  ((bn_dword)0xFFFFFFFF)
#else 
  #error BN_WORD_SIZE must be either to 1, 2 or 4
#endif


/* Data-holding structure: array of DTYPEs */
struct bn
{
  bn_word array[BN_ARRAY_SIZE];
};


/* Initialization functions: */
void bignum_init(struct bn* n);
void bignum_from_int(struct bn* n, bn_dword i);
int  bignum_to_int(struct bn* n);
void bignum_from_string(struct bn* n, char* str, int nbytes);
void bignum_to_string(struct bn* n, char* str, int maxsize);

/* Basic arithmetic operations: */
void bignum_add(struct bn* a, struct bn* b, struct bn* c); /* c = a + b */
void bignum_sub(struct bn* a, struct bn* b, struct bn* c); /* c = a - b */
void bignum_mul(struct bn* a, struct bn* b, struct bn* c); /* c = a * b */
void bignum_div(struct bn* a, struct bn* b, struct bn* c); /* c = a / b */
void bignum_mod(struct bn* a, struct bn* b, struct bn* c); /* c = a % b */
void bignum_divmod(struct bn* a, struct bn* b, struct bn* c, struct bn* d); /* c = a/b, d = a%b */

/* Bitwise operations: */
void bignum_and(struct bn* a, struct bn* b, struct bn* c); /* c = a & b */
void bignum_or(struct bn* a, struct bn* b, struct bn* c);  /* c = a | b */
void bignum_xor(struct bn* a, struct bn* b, struct bn* c); /* c = a ^ b */
void bignum_lshift(struct bn* a, struct bn* b, int nbits); /* b = a << nbits */
void bignum_rshift(struct bn* a, struct bn* b, int nbits); /* b = a >> nbits */

/* Special operators and comparison */
int  bignum_cmp(struct bn* a, struct bn* b);               /* Compare: returns LARGER, EQUAL or SMALLER */
int  bignum_is_zero(struct bn* n);                         /* For comparison with zero */
void bignum_inc(struct bn* n);                             /* Increment: add one to n */
void bignum_dec(struct bn* n);                             /* Decrement: subtract one from n */
void bignum_pow(struct bn* a, struct bn* b, struct bn* c); /* Calculate a^b -- e.g. 2^10 => 1024 */
void bignum_isqrt(struct bn* a, struct bn* b);             /* Integer square root -- e.g. isqrt(5) => 2*/
void bignum_assign(struct bn* dst, struct bn* src);        /* Copy src into dst -- dst := src */


#endif /* #ifndef __BIGNUM_H__ */


