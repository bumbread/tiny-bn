#ifndef BIGNUM__H
#define BIGNUM__H

/*****************************************************************************

Tiny bignum - a small single-header library for arbitrary fixed precision
  arithmetics written in C99.

Licence: public domain.

*****************************************************************************/

#include <stdint.h>
#include <stdbool.h>

#if !defined(bn_assert)
  #include <assert.h>
  #define bn_assert(e,s) assert((e)&&(s))
#endif

#ifndef bn_word_size
  #define bn_word_size 4
#endif

#ifndef bn_array_size
  #define bn_array_size (128 / bn_word_size)
#endif

#if (bn_word_size == 1)
  typedef uint8_t  bn_word;
  typedef uint16_t bn_dword;
  #define bn_word_msb                 ((bn_dword)(0x80))
  #define bn_max_val                  ((bn_dword)0xFF)
#elif (bn_word_size == 2)
  typedef uint16_t bn_word;
  typedef uint32_t bn_dword;
  #define bn_word_msb                 ((bn_dword)(0x8000))
  #define bn_max_val                  ((bn_dword)0xFFFF)
#elif (bn_word_size == 4)
  typedef uint32_t bn_word;
  typedef uint64_t bn_dword;
  #define bn_word_msb                 ((bn_dword)(0x80000000))
  #define bn_max_val                  ((bn_dword)0xFFFFFFFF)
#else 
  #error bn_word_size must be either to 1, 2 or 4
#endif

struct bn
{
  bn_word array[bn_array_size];
};

void bignum_init(struct bn* n);
void bignum_from_int(struct bn* n, bn_dword i);
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
  bn_assert(n, "n is null");

  int i;
  for (i = 0; i < bn_array_size; ++i)
  {
    n->array[i] = 0;
  }
}


void bignum_from_int(struct bn* n, bn_dword i)
{
  bn_assert(n, "n is null");

  bignum_init(n);

  #if (bn_word_size == 1)
    n->array[0] = (i & 0x000000ff);
    n->array[1] = (i & 0x0000ff00) >> 8;
    n->array[2] = (i & 0x00ff0000) >> 16;
    n->array[3] = (i & 0xff000000) >> 24;
  #elif (bn_word_size == 2)
    n->array[0] = (i & 0x0000ffff);
    n->array[1] = (i & 0xffff0000) >> 16;
  #elif (bn_word_size == 4)
    n->array[0] = i;
    bn_dword num_32 = 32;
    bn_dword tmp = i >> num_32; /* bit-shift with U64 operands to force 64-bit results */
    n->array[1] = tmp;
  #endif
}


int bignum_to_int(struct bn* n)
{
  bn_assert(n, "n is null");

  int ret = 0;

  #if (bn_word_size == 1)
    ret += n->array[0];
    ret += n->array[1] << 8;
    ret += n->array[2] << 16;
    ret += n->array[3] << 24;  
  #elif (bn_word_size == 2)
    ret += n->array[0];
    ret += n->array[1] << 16;
  #elif (bn_word_size == 4)
    ret += n->array[0];
  #endif

  return ret;
}

static inline int hexchar__to_int(char a)
{
  if('a' <= a && a <= 'f') return a-'a'+10;
  else return a-'0';
}

void bignum_from_string(struct bn* n, char* str, int nbytes)
{
  bn_assert(n, "n is null");
  bn_assert(str, "str is null");
  bn_assert(nbytes > 0, "nbytes must be positive");
  bn_assert((nbytes & 1) == 0,
            "string format must be in hex -> equal number of bytes");
  bn_assert((nbytes % (sizeof(bn_word) * 2)) == 0,
            "string length must be a multiple"
            " of (sizeof(bn_word) * 2) characters");
  
  bignum_init(n);

  bn_word tmp;                        /* bn_word is defined in bn.h - uint{8,16,32,64}_t */
  int i = nbytes - (2 * bn_word_size); /* index into string */
  int j = 0;                        /* index into array */

  /* reading last hex-byte "MSB" from string first -> big endian */
  /* MSB ~= most significant byte / block ? :) */
  while (i >= 0)
  {
    tmp = 0;
    for(int q=0; q!=2*bn_word_size;++q) {
      int digit = hexchar__to_int(str[i+q]);
      tmp = 16*tmp + digit;
    }
    n->array[j] = tmp;
    i -= (2 * bn_word_size); /* step bn_word_size hex-byte(s) back in the string. */
    j += 1;               /* step one element forward in the array. */
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
  bn_assert(n, "n is null");
  bn_assert(str, "str is null");
  bn_assert(nbytes > 0, "nbytes must be positive");
  bn_assert((nbytes & 1) == 0, "string format must be in hex -> equal number of bytes");

  int j = bn_array_size - 1; /* index into array - reading "MSB" first -> big-endian */
  int i = 0;                 /* index into string representation. */

  /* reading last array-element "MSB" first -> big endian */
  while ((j >= 0) && (nbytes > (i + 1)))
  {
    #if bn_word_size == 1
      str[i+0] = bn__hexhi(n->array[j]);
      str[i+1] = bn__hexlo(n->array[j]);
    #elif bn_word_size == 2
      str[i+0] = bn__hexhi(n->array[j]>>8);
      str[i+1] = bn__hexlo(n->array[j]>>8);
      str[i+2] = bn__hexhi(n->array[j]);
      str[i+3] = bn__hexlo(n->array[j]);
    #elif bn_word_size == 4
      str[i+0] = bn__hexhi(n->array[j]>>16);
      str[i+1] = bn__hexlo(n->array[j]>>16);
      str[i+2] = bn__hexhi(n->array[j]>>8);
      str[i+3] = bn__hexlo(n->array[j]>>8);
      str[i+4] = bn__hexhi(n->array[j]);
      str[i+5] = bn__hexlo(n->array[j]);
    #endif
    i += (2 * bn_word_size); /* step bn_word_size hex-byte(s) forward in the string. */
    j -= 1;               /* step one element back in the array. */
  }

  /* Count leading zeros: */
  j = 0;
  while (str[j] == '0')
  {
    j += 1;
  }
 
  /* Move string j places ahead, effectively skipping leading zeros */ 
  for (i = 0; i < (nbytes - j); ++i)
  {
    str[i] = str[i + j];
  }

  /* Zero-terminate string */
  str[i] = 0;
}


void bignum_dec(struct bn* n)
{
  bn_assert(n, "n is null");

  bn_word tmp; /* copy of n */
  bn_word res;

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
  bn_assert(n, "n is null");

  bn_word res;
  bn_dword tmp; /* copy of n */

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
  bn_assert(a, "a is null");
  bn_assert(b, "b is null");
  bn_assert(c, "c is null");

  bn_dword tmp;
  int carry = 0;
  int i;
  for (i = 0; i < bn_array_size; ++i)
  {
    tmp = (bn_dword)a->array[i] + b->array[i] + carry;
    carry = (tmp > bn_max_val);
    c->array[i] = (tmp & bn_max_val);
  }
}


void bignum_sub(struct bn* a, struct bn* b, struct bn* c)
{
  bn_assert(a, "a is null");
  bn_assert(b, "b is null");
  bn_assert(c, "c is null");

  bn_dword res;
  bn_dword tmp1;
  bn_dword tmp2;
  int borrow = 0;
  int i;
  for (i = 0; i < bn_array_size; ++i)
  {
    tmp1 = (bn_dword)a->array[i] + (bn_max_val + 1); /* + number_base */
    tmp2 = (bn_dword)b->array[i] + borrow;;
    res = (tmp1 - tmp2);
    c->array[i] = (bn_word)(res & bn_max_val); /* "modulo number_base" == "% (number_base - 1)" if number_base is 2^N */
    borrow = (res <= bn_max_val);
  }
}


void bignum_mul(struct bn* a, struct bn* b, struct bn* c)
{
  bn_assert(a, "a is null");
  bn_assert(b, "b is null");
  bn_assert(c, "c is null");

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
        bn_dword intermediate = ((bn_dword)a->array[i] * (bn_dword)b->array[j]);
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
  bn_assert(a, "a is null");
  bn_assert(b, "b is null");
  bn_assert(c, "c is null");

  struct bn current;
  struct bn denom;
  struct bn tmp;

  bignum_from_int(&current, 1);               // int current = 1;
  bignum_assign(&denom, b);                   // denom = b
  bignum_assign(&tmp, a);                     // tmp   = a

  const bn_dword half_max = 1 + (bn_dword)(bn_max_val / 2);
  bool overflow = false;
  while (bignum_cmp(&denom, a) != 1)     // while (denom <= a) {
  {
    if (denom.array[bn_array_size - 1] >= half_max)
    {
      overflow = true;
      break;
    }
    bn__lshift_one_bit(&current);                //   current <<= 1;
    bn__lshift_one_bit(&denom);                  //   denom <<= 1;
  }
  if (!overflow)
  {
    bn__rshift_one_bit(&denom);                  // denom >>= 1;
    bn__rshift_one_bit(&current);                // current >>= 1;
  }
  bignum_init(c);                             // int answer = 0;

  while (!bignum_is_zero(&current))           // while (current != 0)
  {
    if (bignum_cmp(&tmp, &denom) != -1)       //   if (dividend >= denom)
    {
      bignum_sub(&tmp, &denom, &tmp);         //     dividend -= denom;
      bignum_or(c, &current, c);              //     answer |= current;
    }
    bn__rshift_one_bit(&current);                //   current >>= 1;
    bn__rshift_one_bit(&denom);                  //   denom >>= 1;
  }                                           // return answer;
}


void bignum_lshift(struct bn* a, struct bn* b, int nbits)
{
  bn_assert(a, "a is null");
  bn_assert(b, "b is null");
  bn_assert(nbits >= 0, "no negative shifts");

  bignum_assign(b, a);
  /* Handle shift in multiples of word-size */
  const int nbits_pr_word = (bn_word_size * 8);
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
      b->array[i] = (b->array[i] << nbits) | (b->array[i - 1] >> ((8 * bn_word_size) - nbits));
    }
    b->array[i] <<= nbits;
  }
}


void bignum_rshift(struct bn* a, struct bn* b, int nbits)
{
  bn_assert(a, "a is null");
  bn_assert(b, "b is null");
  bn_assert(nbits >= 0, "no negative shifts");
  
  bignum_assign(b, a);
  /* Handle shift in multiples of word-size */
  const int nbits_pr_word = (bn_word_size * 8);
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
      b->array[i] = (b->array[i] >> nbits) | (b->array[i + 1] << ((8 * bn_word_size) - nbits));
    }
    b->array[i] >>= nbits;
  }
  
}


void bignum_mod(struct bn* a, struct bn* b, struct bn* c)
{
  /*
    Take divmod and throw away div part
  */
  bn_assert(a, "a is null");
  bn_assert(b, "b is null");
  bn_assert(c, "c is null");

  struct bn tmp;

  bignum_divmod(a,b,&tmp,c);
}

void bignum_divmod(struct bn* a, struct bn* b, struct bn* c, struct bn* d)
{
  /*
    Puts a%b in d
    and a/b in c

    mod(a,b) = a - ((a / b) * b)

    example:
      mod(8, 3) = 8 - ((8 / 3) * 3) = 2
  */
  bn_assert(a, "a is null");
  bn_assert(b, "b is null");
  bn_assert(c, "c is null");

  struct bn tmp;

  /* c = (a / b) */
  bignum_div(a, b, c);

  /* tmp = (c * b) */
  bignum_mul(c, b, &tmp);

  /* c = a - tmp */
  bignum_sub(a, &tmp, d);
}


void bignum_and(struct bn* a, struct bn* b, struct bn* c)
{
  bn_assert(a, "a is null");
  bn_assert(b, "b is null");
  bn_assert(c, "c is null");

  int i;
  for (i = 0; i < bn_array_size; ++i)
  {
    c->array[i] = (a->array[i] & b->array[i]);
  }
}


void bignum_or(struct bn* a, struct bn* b, struct bn* c)
{
  bn_assert(a, "a is null");
  bn_assert(b, "b is null");
  bn_assert(c, "c is null");

  int i;
  for (i = 0; i < bn_array_size; ++i)
  {
    c->array[i] = (a->array[i] | b->array[i]);
  }
}


void bignum_xor(struct bn* a, struct bn* b, struct bn* c)
{
  bn_assert(a, "a is null");
  bn_assert(b, "b is null");
  bn_assert(c, "c is null");

  int i;
  for (i = 0; i < bn_array_size; ++i)
  {
    c->array[i] = (a->array[i] ^ b->array[i]);
  }
}


int bignum_cmp(struct bn* a, struct bn* b)
{
  bn_assert(a, "a is null");
  bn_assert(b, "b is null");

  int i = bn_array_size;
  do
  {
    i -= 1; /* Decrement first, to start with last array element */
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
  bn_assert(n, "n is null");

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
  bn_assert(a, "a is null");
  bn_assert(b, "b is null");
  bn_assert(c, "c is null");

  struct bn tmp;

  bignum_init(c);

  if (bignum_cmp(b, c) == 0)
  {
    /* Return 1 when exponent is 0 -- n^0 = 1 */
    bignum_inc(c);
  }
  else
  {
    struct bn bcopy;
    bignum_assign(&bcopy, b);

    /* Copy a -> tmp */
    bignum_assign(&tmp, a);

    bignum_dec(&bcopy);
 
    /* Begin summing products: */
    while (!bignum_is_zero(&bcopy))
    {

      /* c = tmp * tmp */
      bignum_mul(&tmp, a, c);
      /* Decrement b by one */
      bignum_dec(&bcopy);

      bignum_assign(&tmp, c);
    }

    /* c = tmp */
    bignum_assign(c, &tmp);
  }
}

void bignum_isqrt(struct bn *a, struct bn* b)
{
  bn_assert(a, "a is null");
  bn_assert(b, "b is null");

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
  bn_assert(dst, "dst is null");
  bn_assert(src, "src is null");

  int i;
  for (i = 0; i < bn_array_size; ++i)
  {
    dst->array[i] = src->array[i];
  }
}


/* Private / Static functions. */
static void bn__rshift_word(struct bn* a, int nwords)
{
  /* Naive method: */
  bn_assert(a, "a is null");
  bn_assert(nwords >= 0, "no negative shifts");

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
  bn_assert(a, "a is null");
  bn_assert(nwords >= 0, "no negative shifts");

  int i;
  /* Shift whole words */
  for (i = (bn_array_size - 1); i >= nwords; --i)
  {
    a->array[i] = a->array[i - nwords];
  }
  /* Zero pad shifted words. */
  for (; i >= 0; --i)
  {
    a->array[i] = 0;
  }  
}


static void bn__lshift_one_bit(struct bn* a)
{
  bn_assert(a, "a is null");

  int i;
  for (i = (bn_array_size - 1); i > 0; --i)
  {
    a->array[i] = (a->array[i] << 1) | (a->array[i - 1] >> ((8 * bn_word_size) - 1));
  }
  a->array[0] <<= 1;
}


static void bn__rshift_one_bit(struct bn* a)
{
  bn_assert(a, "a is null");

  int i;
  for (i = 0; i < (bn_array_size - 1); ++i)
  {
    a->array[i] = (a->array[i] >> 1) | (a->array[i + 1] << ((8 * bn_word_size) - 1));
  }
  a->array[bn_array_size - 1] >>= 1;
}
#endif
#endif


