#ifndef BIGNUM__H
#define BIGNUM__H

/*****************************************************************************

Tiny bignum - a small single-header library for arbitrary fixed precision
  arithmetics written in C99.

Licence: public domain.

*****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#if !defined(bn_assert)
  #include <assert.h>
  #define bn_assert(e) assert(e)
#endif

#ifndef bn_array_size
  #define bn_array_size 32
#endif

#if bn_array_size <= 0
  #error "bn_array_size must be at least 1"
#endif

#define bn_word_msb (UINT64_C(0x80000000))
#define bn_max_val  (UINT64_C(0xFFFFFFFF))

struct bn
{
  uint32_t array[bn_array_size];
};

void bignum_init(struct bn* n);
void bignum_from_int(struct bn* n, uint64_t i);
int  bignum_to_int(struct bn* n);
void bignum_from_string(struct bn* n, char* str, int nbytes);
void bignum_to_string(struct bn* n, char* str, int maxsize);

void bignum_incr(struct bn* n);
void bignum_decr(struct bn* n);
void bignum_add(struct bn* a, struct bn* b, struct bn* c);
void bignum_sub(struct bn* a, struct bn* b, struct bn* c);
void bignum_mul(struct bn* a, struct bn* b, struct bn* c);
void bignum_div(struct bn* a, struct bn* b, struct bn* c);
void bignum_mod(struct bn* a, struct bn* b, struct bn* c);
void bignum_divmod(struct bn* q, struct bn *r, struct bn* lhs, struct bn* rhs);

int  bignum_cmp(struct bn* a, struct bn* b);
int  bignum_is_zero(struct bn* n);
void bignum_assign(struct bn* dst, struct bn* src);

#ifdef bn_implementation

static bool bn_overflow_flag = 0;

void bignum_init(struct bn* n)
{
  bn_assert(n);

  // Todo(bumbread): use bn_memset?
  for(int i = 0; i < bn_array_size; ++i)
  {
    n->array[i] = 0;
  }
}


void bn_from_u64(struct bn* bn, uint64_t n)
{
  bn_assert(bn);
  bignum_init(bn);

  bn->array[0] = n;
  bn->array[1] = n >> 32;
}


int bignum_to_int(struct bn* n)
{
  bn_assert(n);
  return n->array[0];
}

static inline int hexchar__to_int(char a)
{
  if('a' <= a && a <= 'f') return a-'a'+10;
  else return a-'0';
}

void bignum_from_string(struct bn* n, char* str, int nbytes)
{
  bn_assert(n);
  bn_assert(str);
  bn_assert(nbytes > 0);
  bn_assert((nbytes & 1) == 0);
  bn_assert((nbytes % (sizeof(uint32_t) * 2)) == 0);
  
  bignum_init(n);

  const int chars_per_u32 = 2*sizeof(uint32_t);

  uint32_t tmp;
  int i = nbytes - chars_per_u32;
  int j = 0;
  while (i >= 0)
  {
    tmp = 0;
    for(int q=0; q!=chars_per_u32;++q) {
      int digit = hexchar__to_int(str[i+q]);
      tmp = 16*tmp + digit;
    }

    n->array[j] = tmp;
    i -= chars_per_u32;
    j += 1;
  }
}

static inline char bn__hexlo(uint8_t b)
{
  char q=(char)(b&0x0F);
  if(q>=10) q+='a';
  else q+='0';
  return q;
}

static inline char bn__hexhi(uint8_t b)
{
  char q=(char)(b>>4);
  if(q>=10)q+='a';
  else q+='0';
  return q;
}

void bignum_to_string(struct bn* n, char* str, int nbytes)
{
  bn_assert(n);
  bn_assert(str);
  bn_assert(nbytes > 0);
  bn_assert((nbytes & 1) == 0);

  int j = bn_array_size - 1;
  int i = 0;

  while ((j >= 0) && (nbytes > (i + 1)))
  {
    str[i+0] = bn__hexhi(n->array[j]>>16);
    str[i+1] = bn__hexlo(n->array[j]>>16);
    str[i+2] = bn__hexhi(n->array[j]>>8);
    str[i+3] = bn__hexlo(n->array[j]>>8);
    str[i+4] = bn__hexhi(n->array[j]);
    str[i+5] = bn__hexlo(n->array[j]);
    i += (2 * sizeof(uint32_t));
    j -= 1;
  }

  j = 0;
  while (str[j] == '0')
  {
    j += 1;
  }
 
  for (i = 0; i < (nbytes - j); ++i)
  {
    str[i] = str[i + j];
  }

  str[i] = 0;
}


void bignum_decr(struct bn* n)
{
  bn_assert(n);

  bool carry = 1;
  for(int i = 0; i != bn_array_size; ++i)
  {
    n->array[i] -= 1;

    // Note(bumbread): If decrement overflowed, the new
    // value would be 0xFFFFFFFF
    if(n->array[i] != 0xFFFFFFFF) {
      carry = 0;
      break;
    }
  }
  
  bn_overflow_flag = (carry==1);
}


void bignum_incr(struct bn* n)
{
  bn_assert(n);

  bool carry = 1;
  for(int i = 0; i != bn_array_size; ++i)
  {
    n->array[i] += 1;

    // Note(bumbread): If increment overflowed, the new
    // value would be 0x00000000
    if(n->array[i] != 0) {
      carry = 0;
      break;
    }
  }
  
  bn_overflow_flag = (carry==1);
}


void bignum_add(struct bn* res, struct bn* lhs, struct bn* rhs)
{
  bn_assert(res);
  bn_assert(lhs);
  bn_assert(rhs);

  uint64_t tmp;
  uint64_t carry = 0;
  int i;
  for (i = 0; i < bn_array_size; ++i)
  {
    tmp = (uint64_t)lhs->array[i] + (uint64_t)rhs->array[i] + carry;
    res->array[i] = (uint32_t)(tmp & 0xFFFFFFFF);
    carry = tmp >> 32;
  }

  bn_overflow_flag = (carry!=0);
}


void bignum_sub(struct bn* res, struct bn* lhs, struct bn* rhs)
{
  bn_assert(res);
  bn_assert(lhs);
  bn_assert(rhs);

  int borrow = 0;
  for (int i = 0; i < bn_array_size; ++i) {
    int old_borrow=borrow;
    borrow = (rhs->array[i] > lhs->array[i]) ? 1 : 0;
    res->array[i] = lhs->array[i] - rhs->array[i] - old_borrow;
  }

  bn_overflow_flag = (borrow!=0);
}


void bignum_mul(struct bn* res, struct bn* lhs, struct bn* rhs)
{
  bn_assert(res);
  bn_assert(lhs);
  bn_assert(rhs);

  struct bn sum;
  bignum_init(&sum);

  uint64_t carry =0;

  for(int i = 0; i < bn_array_size; ++i)
  {
    struct bn row;
    bignum_init(&row);

    carry = 0;
    for (int j = 0; j < bn_array_size - i; ++j)
    {
      uint64_t ld = (uint64_t)lhs->array[j];
      uint64_t rd = (uint64_t)rhs->array[i];
      uint64_t tmp = carry + ld + rd;
      row.array[i+j] += (uint32_t)(tmp & 0xFFFFFFFF);
      carry = tmp >> 32;
    }

    bignum_add(&sum, &sum, &row);
    bn_overflow_flag |= (carry != 0);
  }

  bignum_assign(res, &sum);
}

static uint64_t
bignum__get_ndigits(struct bn *b)
{
  uint64_t ndigits = bn_array_size;
  do {
    if(b->array[ndigits-1] != 0) {
      break;
    }
    ndigits --;
  } while(ndigits != 1);
  return ndigits;
}

static uint32_t
bignum_divmod__getdigit(struct bn *r, struct bn *lhs, struct bn *rhs)
{
  bn_assert(r);
  bn_assert(lhs);
  bn_assert(rhs);

  uint64_t ldigits = bignum__get_ndigits(lhs);
  uint64_t rdigits = bignum__get_ndigits(rhs);

  if(ldigits == 1 && rdigits == 1) {
    uint32_t left = lhs->array[0];
    uint32_t right = rhs->array[0];

    uint32_t q0 = left / right;
    uint32_t r0 = left % right;

    bignum_from_int(r, r0);
    return q0;
  }
  else {
    struct bn divident;
    bignum_assign(&divident, lhs);

    uint32_t quotient = 0;
    while(bignum_cmp(&divident, rhs) > 0) {
      bignum_sub(&divident, &divident, rhs);
      quotient += 1;
    }

    bignum_assign(&divident, r);
    return quotient;
  }
}

static void
bignum__append_digit(struct bn *res, struct bn *num, uint32_t digit)
{
  uint64_t i = bn_array_size;
  if(i != 1) do {
    i --;
    res->array[i] = num->array[i-1];
  } while(i != 1);
  res->array[0] = digit;
}

void
bignum_divmod(struct bn* quot, struct bn *rem, struct bn* lhs, struct bn* rhs)
{
  bn_assert(lhs);
  bn_assert(rhs);
  bn_assert(quot);
  bn_assert(rem);

  if(bignum_is_zero(rhs)) {
    bn_assert(false);
    return;
  }

  if(bignum_cmp(lhs, rhs) == -1) {
    // Note(bumbread/2021-10-21): this may be wrong, when the pointers
    // are aliased. Assignment to rem from lhs may overwrite rhs,
    // leaving the rest of procedure wrong.
    bignum_assign(rem, lhs);
    bignum_assign(quot, rhs);
    return;
  }

  struct bn divident;
  struct bn quotient;
  struct bn remainder;

  bignum_from_int(&quotient, 0);
  bignum_from_int(&remainder, 0);
  uint64_t ldigits = bignum__get_ndigits(lhs);
  uint64_t rdigits = bignum__get_ndigits(rhs);
  
  {
    // Initialise remainder to be `rdigits-1` digits of the
    // lhs number.
    uint64_t i = rdigits - 1;
    if(i != 0) do {
      i --;
      remainder.array[i] = lhs->array[i+1];
    } while(i != 0);
  }

  uint64_t di = ldigits-1;
  for(uint64_t i = 0; i <= rdigits-ldigits; ++i) {
    bignum__append_digit(&divident, &remainder, lhs->array[di--]);
    uint32_t digit = bignum_divmod__getdigit(&remainder, &divident, rhs);
    bignum__append_digit(&quotient, &quotient, digit);
  }
}


int bignum_cmp(struct bn* a, struct bn* b)
{
  bn_assert(a);
  bn_assert(b);

  int i = bn_array_size;
  do
  {
    i -= 1;
    if (a->array[i] > b->array[i])
    {
      return 1;
    }
    else if (a->array[i] < b->array[i])
    {
      return -1;
    }
  }
  while (i != 0);

  return 0;
}


int bignum_is_zero(struct bn* n)
{
  bn_assert(n);

  int i;
  for (i = 0; i < bn_array_size; ++i)
  {
    if (n->array[i])
    {
      return 0;
    }
  }

  return 1;
}

void bignum_assign(struct bn* dst, struct bn* src)
{
  bn_assert(dst);
  bn_assert(src);

  int i;
  for (i = 0; i < bn_array_size; ++i)
  {
    dst->array[i] = src->array[i];
  }
}

#endif
#endif


