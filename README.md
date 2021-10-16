![CI](https://github.com/kokke/tiny-bignum-c/workflows/CI/badge.svg)

# tiny-bignum-c - A small multiple-precision integer implementation in C

## Description

Tiny Bignum is a small portable [Arbitrary-precision](https://en.wikipedia.org/wiki/Arbitrary-precision_arithmetic)
  unsigned integer arithmetic in C99, for calculating with large numbers. The
  actual precision of the big integer type used in the library is unlimited in
  practice, but in fact it is limited to 128 bytes. That limit could be changed
  by defining a macro.

The library offers the choice of `uint32_t`, `uint16_t` or `uint8_t` as its
  underlying "digit" representation. By default the library uses `uint32_t`s.

The library offers no overflow handling for when the integer grows over the
  size of the underlying representation. Thus all operations truncate.

No dynamic memory management is utilized, `stdio.h` is used for testing
  functions parsing to and from hex-strings (`sprintf`, `sscanf`).

## Design goals

- C89/C99/C++ compatibility
- Able to be included in many different projects
- Self-contained. The library has no external and CRT dependencies.
- Code clarity. The code is sprinkled with holy comments.

Goals/Functions provided by the library:

- Overflow signaling
- Basic arithmetic, Bitwise operators, GCD, LCM

## Current status

- GCD, LCM are not implemented
- Overflow handling is not implemented
- The library has poor support for C++ and C89.

## Usage

This is the datatype used for the operations.

```C
struct bn
{
  bn_word array[BN_ARRAY_SIZE];
};
```

The API of the library:

```C
/* Initialization functions: */
void bignum_init(struct bn* n); /* n gets zero-initialized */
void bignum_from_int(struct bn* n, DTYPE_TMP i);
int  bignum_to_int(struct bn* n);

/* NOTE: these functions use HEX representation of str! */
void bignum_from_string(struct bn* n, char* str, int nbytes);
void bignum_to_string(struct bn* n, char* str, int maxsize);

/* Basic arithmetic operations: */
void bignum_add(struct bn* a, struct bn* b, struct bn* c);
void bignum_sub(struct bn* a, struct bn* b, struct bn* c);
void bignum_mul(struct bn* a, struct bn* b, struct bn* c);
void bignum_div(struct bn* a, struct bn* b, struct bn* c);
void bignum_mod(struct bn* a, struct bn* b, struct bn* c);
void bignum_divmod(struct bn* a, struct bn* b, struct bn* c, struct bn* d);

/* Bitwise operations: */
void bignum_and(struct bn* a, struct bn* b, struct bn* c);
void bignum_or(struct bn* a, struct bn* b, struct bn* c);
void bignum_xor(struct bn* a, struct bn* b, struct bn* c);
void bignum_lshift(struct bn* a, struct bn* b, int nbits);
void bignum_rshift(struct bn* a, struct bn* b, int nbits);

/* Special operators and comparison */
int  bignum_cmp(struct bn* a, struct bn* b);
int  bignum_is_zero(struct bn* n);
void bignum_inc(struct bn* n);
void bignum_dec(struct bn* n);
void bignum_pow(struct bn* a, struct bn* b, struct bn* c); /* exponentiation */
void bignum_isqrt(struct bn* a, struct bn* b);      /* square root */
void bignum_assign(struct bn* dst, struct bn* src); /* copies the data */
```

### Changing the digit size

You can change the "digit" of the underlying representation to be either
1 byte, 2 bytes or 4 bytes. To change the size of the digit, use one of the
following lines

```c
#define BN_WORD_SIZE 1
#define BN_WORD_SIZE 2
#define BN_WORD_SIZE 3
```

In EVERY file that includes `bn.h`. Alternatively add `-DBN_WORD_SIZE=n`
compiler option for every compiled file.

### Changing the precision

You can change the max number of digits in a single bigint (in terms of digits)
by defining `BN_ARRAY_SIZE`. By default BN_ARRAY_SIZE is defined in such a way
as to make the precision of 1024 bits.

If you change the precision, you have to have the same define in EVERY file
that includes `bn.h`. Alternatively add `-DBN_WORD_SIZE=n` compiler option
for every compiled file.

## License

All material in this repository is in the public domain.

