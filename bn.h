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

void bignum_add(struct bn* a, struct bn* b, struct bn* c);
void bignum_sub(struct bn* a, struct bn* b, struct bn* c);
void bignum_mul(struct bn* a, struct bn* b, struct bn* c);
void bignum_div(struct bn* a, struct bn* b, struct bn* c);
void bignum_mod(struct bn* a, struct bn* b, struct bn* c);
void bignum_divmod(struct bn* a, struct bn* b, struct bn* c, struct bn* d);

void bignum_and(struct bn* a, struct bn* b, struct bn* c);
void bignum_or(struct bn* a, struct bn* b, struct bn* c);
void bignum_xor(struct bn* a, struct bn* b, struct bn* c);
void bignum_lshift(struct bn* a, struct bn* b, int nbits);
void bignum_rshift(struct bn* a, struct bn* b, int nbits);

int  bignum_cmp(struct bn* a, struct bn* b);
int  bignum_is_zero(struct bn* n);
void bignum_incr(struct bn* n);
void bignum_decr(struct bn* n);
void bignum_pow(struct bn* a, struct bn* b, struct bn* c);
void bignum_isqrt(struct bn* a, struct bn* b);
void bignum_assign(struct bn* dst, struct bn* src);

#ifdef bn_implementation

static void bn__lshift_one_bit(struct bn* a);
static void bn__rshift_one_bit(struct bn* a);
static void bn__lshift_word(struct bn* a, int nwords);
static void bn__rshift_word(struct bn* a, int nwords);

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


void bignum_dec(struct bn* n)
{
  bn_assert(n);

  uint32_t tmp;
  uint32_t res;

  int i;
  for (i = 0; i < bn_array_size; ++i)
  {
    tmp = n->array[i];
    res = tmp - 1;
    n->array[i] = res;

    if (!(res > tmp))
    {
      break;
    }
  }
}


void bignum_inc(struct bn* n)
{
  bn_assert(n);

  uint32_t res;
  uint64_t tmp;

  int i;
  for (i = 0; i < bn_array_size; ++i)
  {
    tmp = n->array[i];
    res = tmp + 1;
    n->array[i] = res;

    if (res > tmp)
    {
      break;
    }
  }
}


void bignum_add(struct bn* a, struct bn* b, struct bn* c)
{
  bn_assert(a);
  bn_assert(b);
  bn_assert(c);

  uint64_t tmp;
  int carry = 0;
  int i;
  for (i = 0; i < bn_array_size; ++i)
  {
    tmp = (uint64_t)a->array[i] + b->array[i] + carry;
    carry = (tmp > bn_max_val);
    c->array[i] = (tmp & bn_max_val);
  }
}


void bignum_sub(struct bn* a, struct bn* b, struct bn* c)
{
  bn_assert(a);
  bn_assert(b);
  bn_assert(c);

  uint64_t res;
  uint64_t tmp1;
  uint64_t tmp2;
  int borrow = 0;
  int i;
  for (i = 0; i < bn_array_size; ++i)
  {
    tmp1 = (uint64_t)a->array[i] + (bn_max_val + 1);
    tmp2 = (uint64_t)b->array[i] + borrow;;
    res = (tmp1 - tmp2);
    c->array[i] = (uint32_t)(res & bn_max_val);
    borrow = (res <= bn_max_val);
  }
}


void bignum_mul(struct bn* a, struct bn* b, struct bn* c)
{
  bn_assert(a);
  bn_assert(b);
  bn_assert(c);

  struct bn row;
  struct bn tmp;
  int i, j;

  bignum_init(c);

  for (i = 0; i < bn_array_size; ++i)
  {
    bignum_init(&row);

    for (j = 0; j < bn_array_size; ++j)
    {
      if (i + j < bn_array_size)
      {
        bignum_init(&tmp);
        uint64_t intermediate = ((uint64_t)a->array[i] * (uint64_t)b->array[j]);
        bignum_from_int(&tmp, intermediate);
        bn__lshift_word(&tmp, i + j);
        bignum_add(&tmp, &row, &row);
      }
    }
    bignum_add(c, &row, c);
  }
}


void bignum_div(struct bn* a, struct bn* b, struct bn* c)
{
  bn_assert(a);
  bn_assert(b);
  bn_assert(c);

  struct bn current;
  struct bn denom;
  struct bn tmp;

  bignum_from_int(&current, 1);
  bignum_assign(&denom, b);
  bignum_assign(&tmp, a);

  const uint64_t half_max = 1 + (uint64_t)(bn_max_val / 2);
  bool overflow = false;
  while (bignum_cmp(&denom, a) != 1)
  {
    if (denom.array[bn_array_size - 1] >= half_max)
    {
      overflow = true;
      break;
    }
    bn__lshift_one_bit(&current);
    bn__lshift_one_bit(&denom);
  }
  if (!overflow)
  {
    bn__rshift_one_bit(&denom);
    bn__rshift_one_bit(&current);
  }
  bignum_init(c);

  while (!bignum_is_zero(&current))
  {
    if (bignum_cmp(&tmp, &denom) != -1)
    {
      bignum_sub(&tmp, &denom, &tmp);
      bignum_or(c, &current, c);
    }
    bn__rshift_one_bit(&current);
    bn__rshift_one_bit(&denom);
  }
}


void bignum_lshift(struct bn* a, struct bn* b, int nbits)
{
  bn_assert(a);
  bn_assert(b);
  bn_assert(nbits >= 0);

  bignum_assign(b, a);

  const int nbits_pr_word = (sizeof(uint32_t) * 8);
  int nwords = nbits / nbits_pr_word;
  if (nwords != 0)
  {
    bn__lshift_word(b, nwords);
    nbits -= (nwords * nbits_pr_word);
  }

  if (nbits != 0)
  {
    int i;
    for (i = (bn_array_size - 1); i > 0; --i)
    {
      b->array[i] = (b->array[i] << nbits) | (b->array[i - 1] >> ((8 * sizeof(uint32_t)) - nbits));
    }
    b->array[i] <<= nbits;
  }
}


void bignum_rshift(struct bn* a, struct bn* b, int nbits)
{
  bn_assert(a);
  bn_assert(b);
  bn_assert(nbits >= 0);
  
  bignum_assign(b, a);
  
  const int nbits_pr_word = (sizeof(uint32_t) * 8);
  int nwords = nbits / nbits_pr_word;
  if (nwords != 0)
  {
    bn__rshift_word(b, nwords);
    nbits -= (nwords * nbits_pr_word);
  }

  if (nbits != 0)
  {
    int i;
    for (i = 0; i < (bn_array_size - 1); ++i)
    {
      b->array[i] = (b->array[i] >> nbits) | (b->array[i + 1] << ((8 * sizeof(uint32_t)) - nbits));
    }
    b->array[i] >>= nbits;
  }
  
}


void bignum_mod(struct bn* a, struct bn* b, struct bn* c)
{
  bn_assert(a);
  bn_assert(b);
  bn_assert(c);

  struct bn tmp;

  bignum_divmod(a,b,&tmp,c);
}

void bignum_divmod(struct bn* a, struct bn* b, struct bn* c, struct bn* d)
{
  bn_assert(a);
  bn_assert(b);
  bn_assert(c);

  struct bn tmp;

  bignum_div(a, b, c);
  bignum_mul(c, b, &tmp);
  bignum_sub(a, &tmp, d);
}


void bignum_and(struct bn* a, struct bn* b, struct bn* c)
{
  bn_assert(a);
  bn_assert(b);
  bn_assert(c);

  int i;
  for (i = 0; i < bn_array_size; ++i)
  {
    c->array[i] = (a->array[i] & b->array[i]);
  }
}


void bignum_or(struct bn* a, struct bn* b, struct bn* c)
{
  bn_assert(a);
  bn_assert(b);
  bn_assert(c);

  int i;
  for (i = 0; i < bn_array_size; ++i)
  {
    c->array[i] = (a->array[i] | b->array[i]);
  }
}


void bignum_xor(struct bn* a, struct bn* b, struct bn* c)
{
  bn_assert(a);
  bn_assert(b);
  bn_assert(c);

  int i;
  for (i = 0; i < bn_array_size; ++i)
  {
    c->array[i] = (a->array[i] ^ b->array[i]);
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


void bignum_pow(struct bn* a, struct bn* b, struct bn* c)
{
  bn_assert(a);
  bn_assert(b);
  bn_assert(c);

  struct bn tmp;

  bignum_init(c);

  if (bignum_cmp(b, c) == 0)
  {
    bignum_inc(c);
  }
  else
  {
    struct bn bcopy;
    bignum_assign(&bcopy, b);
    bignum_assign(&tmp, a);
    bignum_dec(&bcopy);
    while (!bignum_is_zero(&bcopy))
    {
      bignum_mul(&tmp, a, c);
      bignum_dec(&bcopy);
      bignum_assign(&tmp, c);
    }

    bignum_assign(c, &tmp);
  }
}

void bignum_isqrt(struct bn *a, struct bn* b)
{
  bn_assert(a);
  bn_assert(b);

  struct bn low, high, mid, tmp;

  bignum_init(&low);
  bignum_assign(&high, a);
  bignum_rshift(&high, &mid, 1);
  bignum_inc(&mid);

  while (bignum_cmp(&high, &low) > 0) 
  {
    bignum_mul(&mid, &mid, &tmp);
    if (bignum_cmp(&tmp, a) > 0) 
    {
      bignum_assign(&high, &mid);
      bignum_dec(&high);
    }
    else 
    {
      bignum_assign(&low, &mid);
    }
    bignum_sub(&high,&low,&mid);
    bn__rshift_one_bit(&mid);
    bignum_add(&low,&mid,&mid);
    bignum_inc(&mid);
  }
  bignum_assign(b,&low);
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

static void bn__rshift_word(struct bn* a, int nwords)
{
  int i;
  if (nwords >= bn_array_size)
  {
    for (i = 0; i < bn_array_size; ++i)
    {
      a->array[i] = 0;
    }
    return;
  }

  for (i = 0; i < bn_array_size - nwords; ++i)
  {
    a->array[i] = a->array[i + nwords];
  }
  for (; i < bn_array_size; ++i)
  {
    a->array[i] = 0;
  }
}


static void bn__lshift_word(struct bn* a, int nwords)
{
  int i;
  for (i = (bn_array_size - 1); i >= nwords; --i)
  {
    a->array[i] = a->array[i - nwords];
  }
  for (; i >= 0; --i)
  {
    a->array[i] = 0;
  }  
}


static void bn__lshift_one_bit(struct bn* a)
{
  int i;
  for (i = (bn_array_size - 1); i > 0; --i)
  {
    a->array[i] = (a->array[i] << 1) | (a->array[i - 1] >> ((8 * sizeof(uint32_t)) - 1));
  }
  a->array[0] <<= 1;
}


static void bn__rshift_one_bit(struct bn* a)
{
  int i;
  for (i = 0; i < (bn_array_size - 1); ++i)
  {
    a->array[i] = (a->array[i] >> 1) | (a->array[i + 1] << ((8 * sizeof(uint32_t)) - 1));
  }
  a->array[bn_array_size - 1] >>= 1;
}
#endif
#endif


