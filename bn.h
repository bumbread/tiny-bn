#ifndef BIGNUM__H
#define BIGNUM__H

/*****************************************************************************

Tiny bignum - a small single-header library for arbitrary fixed precision
  arithmetics written in C99.

Licence: public domain.

*****************************************************************************/

#include <stdint.h>
#include <inttypes.h>

#if !defined(bn_assert)
  #include <assert.h>
  #define bn_assert(e) assert(e)
#endif

#ifndef bn_array_size
  #define bn_array_size 32
#endif

#if bn_array_size <= 1
  #error "bn_array_size must be at least 2"
#endif

#define bn_word_msb (UINT64_C(0x80000000))
#define bn_max_val  (UINT64_C(0xFFFFFFFF))

typedef struct Bignum Bignum;
struct Bignum
{
  uint32_t array[bn_array_size];
};

/*****************************************************************************
  Initialization functions.
  The  first  argument to all these functions is a pointer to the bignum to be
  initialized.

  1. `bignum_init`
    Takes  in  a  reference  to  Bignum  variable  and initializes it to zero.
    
    Alternatively you can initialize Bignum variables with
    `variable = {0};` in C or `variable = {};` in C++.

  2. `bignum_from_u64`
    Takes  64-bit unsigned integer and initializes Bignum with the same value.

  3. `bignum_from_hex`
    Takes  a  string  with  HEX  representation  of  a bignum, and initializes
    a  Bignum variable with the value encoded in the string. The string has to
    be multiple of 8 characters.

    `maxsize`  has  to  represent the actual size of the string (or less), and
    be a multiple of 8.

  4. `bignum_assign`
    Takes  initalized  `src`  Bignum, and copies it's value to `dst` variable,
    initializing it.

*****************************************************************************/
void bignum_init(Bignum* n);
void bignum_from_u64(Bignum* n, uint64_t i);
void bignum_from_hex(Bignum* n, char const* str, int maxsize);
void bignum_assign(Bignum* dst, Bignum const* src);


/*****************************************************************************
  Getting the value out of bignum.

  1. `u64_from_bignum`
    Takes an initialized Bignum integer and converts it to u64. If the value
    of the Bignum is not representable by u64, it is truncated to the lowest
    64  bits  and  the  Overflow Flag is set. For more information about the
    overflow flag see the section below.

  2. `hex_from_bignum`
    Takes  a  finite  string  of  hex  digits  and  converts  it  to a bignum.
    `maxsize`  must  be no greater than the size of the `str`. The digits will
    be   written  in  the  big-endian  format,  with  the  lowest-order  digit
    necessarily being written in the (maxsize-1)-th's character of the string.
*****************************************************************************/
uint64_t u64_from_bignum(Bignum const* n);
void hex_from_bignum(Bignum const* n, char* str, int maxsize);


/*****************************************************************************
  Overflow management

  1. `bignum_reset_overflow_flag`
    Resets  overflow  flag,  meaning that it will be treated as if no overflow
      had occured.

  2. `bignum_is_overflow`
    Returns the value of the overflow flag. Zero, if the overflow wasn't
      occured, and non-zero value if it has.
*****************************************************************************/
void bignum_reset_overflow_flag(void);
int bignum_is_overflow(void);

/*****************************************************************************
  Comparison functions

  1. `bignum_cmp`
    Returns negative value, if `a` is smaller than `b`
            0,              if `a` and `b` are equal
            positive value, if `a` is greater than `b`

  2. `bignum_greater`
    Returns non-zero value if `a` is greater than `b`
    Returns zero, if `a` is less than or equal to `b`

  3. `bignum_less`
    Returns non-zero value if `a` is less than `b`
    Returns zero, if `a` is greater than or equal to `b`

  4. `bignum_geq`
    Returns non-zero value if `a` is greater than or equal to `b`
    Returns zero if `a` is less than `b`

  5. `bignum_leq`
    Returns non-zero value if `a` is less than or equal to `b`
    Returns zero if `a` is greater than `b`

  6. `bignum_is_zero`
    Returns non-zero value if `n` is zero
    Returns zero if `n` is non-zero.
*****************************************************************************/
int  bignum_cmp(Bignum const* a, Bignum const* b);
int  bignum_greater(Bignum const* a, Bignum const* b);
int  bignum_less(Bignum const* a, Bignum const* b);
int  bignum_geq(Bignum const* a, Bignum const* b);
int  bignum_leq(Bignum const* a, Bignum const* b);
int  bignum_equal(Bignum const* a, Bignum const* b);
int  bignum_is_zero(Bignum const* n);


/*****************************************************************************
  Arithmetic functions
  
  The result pointer of the functions is allowed to point to the same object
  as the left-hand side operand of the corresponding operation.

  If the result of the operation overflows (becomes unrepresentable by current
  size), for example being higher than 2**(4*bn_array_size), or less than zero
  then  the overflow flag is set, and you can check it with bignum_is_overflow
  function.

  1. `bignum_incr`
    Increases  the  value of `n` by 1. If `n` overflows, overflow flag is set.

  2. `bignum_decr`
    Decreases   the  value of `n` by 1.
  
  3. `bignum_add`
    Adds  the  values  of  `b`  and `c` and stores the resulting value in `a`.

  4. `bignum_sub`
    Sutracts  the values of `b` and `c` and stores the resulting value in `a`.

  5. `bignum_mul`
    Multiplies  the numbers `b` and `c` and stores the resulting value in `a`.

  6. `bignum_div`
    Divides the numbers `a` by `b`, stores the quotient of the division in
    `q`, and the remainder in `r`.
*****************************************************************************/
void bignum_incr(Bignum* n);
void bignum_decr(Bignum* n);
void bignum_add(Bignum* a, Bignum const* b, Bignum const* c);
void bignum_sub(Bignum* a, Bignum const* b, Bignum const* c);
void bignum_mul(Bignum* a, Bignum const* b, Bignum const* c);
void bignum_divmod(Bignum* q, Bignum *r, Bignum const* a, Bignum const* b);

/*****************************************************************************
  END OF HEADING
*****************************************************************************/



#ifdef bn_implementation

static int bn_overflow_flag = 0;

void bignum_init(Bignum* n)
{
  bn_assert(n);

  // Todo(bumbread): use bn_memset?
  for(int i = 0; i < bn_array_size; ++i)
  {
    n->array[i] = 0;
  }
}

void bignum_from_u64(Bignum* bn, uint64_t n)
{
  bn_assert(bn);
  bignum_init(bn);

  bn->array[0] = (uint32_t)n;
  bn->array[1] = n >> 32;
}

static inline int hexchar__to_int(char a)
{
  if('a' <= a && a <= 'f') return a-'a'+10;
  else return a-'0';
}

void bignum_from_hex(Bignum* n, char const* str, int maxsize)
{
  bn_assert(n);
  bn_assert(str);
  bn_assert(maxsize > 0);
  bn_assert(maxsize%2 == 0);
  
  bignum_init(n);

  const int chars_per_u32 = 2*sizeof(uint32_t);

  uint32_t tmp;
  int i = maxsize - chars_per_u32;
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

void bignum_assign(Bignum* dst, Bignum const* src)
{
  bn_assert(dst);
  bn_assert(src);

  int i;
  for (i = 0; i < bn_array_size; ++i)
  {
    dst->array[i] = src->array[i];
  }
}



uint64_t u64_from_bignum(Bignum const* n)
{
  bn_assert(n);
  return n->array[0] | (n->array[1] << 1);
}

static inline char bn__hexlo(uint32_t b)
{
  char q=(char)(b&0x0F);
  if(q>=10) q+='a';
  else q+='0';
  return q;
}

static inline char bn__hexhi(uint32_t b)
{
  char q=(char)(b>>4);
  if(q>=10)q+='a';
  else q+='0';
  return q;
}

void hex_from_bignum(Bignum const* n, char* str, int maxsize)
{
  bn_assert(n);
  bn_assert(str);
  bn_assert(maxsize > 0);
  bn_assert(maxsize%8 == 0);

  int j = bn_array_size - 1;
  int i = 0;

  while ((j >= 0) && (maxsize > (i + 1)))
  {
    str[i+0] = bn__hexhi(n->array[j]>>24);
    str[i+1] = bn__hexlo(n->array[j]>>24);
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
 
  for (i = 0; i < maxsize-j; ++i)
  {
    str[i] = str[i + j];
  }

  str[i] = 0;
}



void bignum_reset_overflow_flag(void)
{
  bn_overflow_flag = 0;
}

int bignum_is_overflow(void)
{
  return bn_overflow_flag;
}



void bignum_incr(Bignum* n)
{
  bn_assert(n);
  int overflow = 0;
  for(int i = 0; i != bn_array_size; ++i)
  {
    n->array[i] += 1;
    // Note(bumbread): If increment overflowed, the new
    // value would be 0x00000000
    if(n->array[i] == 0) {
      overflow = 1;
    }
    else {
      break;
    }
  }
  bn_overflow_flag |= overflow;
}

void bignum_decr(Bignum* n)
{
  bn_assert(n);
  int borrow = 0;
  for(int i = 0; i != bn_array_size; ++i)
  {
    n->array[i] -= 1;
    // Note(bumbread): If decrement overflowed, the new
    // value would be 0xFFFFFFFF
    if(n->array[i] == 0xFFFFFFFF) {
      borrow = 1;
    }
    else {
      break;
    }
  }
  bn_overflow_flag |= borrow;
}

void bignum_add(Bignum* res, Bignum const* lhs, Bignum const* rhs)
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


void bignum_sub(Bignum* res, Bignum const* lhs, Bignum const* rhs)
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


void bignum_mul(Bignum* res, Bignum const* lhs, Bignum const* rhs)
{
  bn_assert(res);
  bn_assert(lhs);
  bn_assert(rhs);

  Bignum sum;
  bignum_init(&sum);

  uint64_t carry =0;

  for(int i = 0; i < bn_array_size; ++i)
  {
    Bignum row;
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
bignum__get_ndigits(Bignum const *b)
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
bignum_divmod__getdigit(Bignum *r, Bignum const *lhs, Bignum const *rhs)
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

    bignum_from_u64(r, r0);
    return q0;
  }
  else {
    Bignum divident;
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
bignum__append_digit(Bignum *res, Bignum const *num, uint32_t digit)
{
  uint64_t i = bn_array_size;
  if(i != 1) do {
    i --;
    res->array[i] = num->array[i-1];
  } while(i != 1);
  res->array[0] = digit;
}

void
bignum_divmod(Bignum* quot, Bignum *rem, Bignum const* lhs, Bignum const* rhs)
{
  bn_assert(lhs);
  bn_assert(rhs);
  bn_assert(quot);
  bn_assert(rem);

  if(bignum_is_zero(rhs)) {
    bn_assert(0);
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

  Bignum divident;
  Bignum quotient = {0};
  Bignum remainder = {0};

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

int bignum_cmp(Bignum const* a, Bignum const* b)
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

int bignum_greater(Bignum const* a, Bignum const* b)
{
  bn_assert(a);
  bn_assert(b);

  int i = bn_array_size;
  do {
    -- i;
    if(a->array[i] <= b->array[i]) {
      return 0;
    }
  } while(i!=0);

  return 1;
}

int bignum_less(Bignum const* a, Bignum const* b)
{
  bn_assert(a);
  bn_assert(b);

  int i = bn_array_size;
  do {
    -- i;
    if(a->array[i] >= b->array[i]) {
      return 0;
    }
  } while(i!=0);

  return 1;
}

int bignum_geq(Bignum const* a, Bignum const* b)
{
  bn_assert(a);
  bn_assert(b);

  int i = bn_array_size;
  do {
    -- i;
    if(a->array[i] < b->array[i]) {
      return 0;
    }
  } while(i!=0);

  return 1;
}

int bignum_leq(Bignum const* a, Bignum const* b)
{
  bn_assert(a);
  bn_assert(b);

  int i = bn_array_size;
  do {
    -- i;
    if(a->array[i] > b->array[i]) {
      return 0;
    }
  } while(i!=0);

  return 1;
}

int bignum_equal(Bignum const* a, Bignum const* b)
{
  bn_assert(a);
  bn_assert(b);

  int i = bn_array_size;
  do {
    -- i;
    if(a->array[i] != b->array[i]) {
      return 0;
    }
  } while(i!=0);

  return 1;
}

int bignum_is_zero(Bignum const* n)
{
  bn_assert(n);

  int i = bn_array_size;
  do {
    -- i;
    if(n->array[i] != 0) {
      return 0;
    }
  } while(i!=0);

  return 1;
}

#endif
#endif


