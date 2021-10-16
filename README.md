![CI](https://github.com/kokke/tiny-bignum-c/workflows/CI/badge.svg)

# tiny-bignum-c - A small multiple-precision integer implementation in C

This library is a fork of kokke's [tiny-bignum-c](https://github.com/kokke/tiny-bignum-c) library. This fork introduces several changes into the original library:

- It is a single-header library that can be used in C (>= C99) and C++.
- No CRT dependency (except assert macro, that could be disabled)
- Removes UB
- Adds prefixes to the library's functions, defines et cetera
- Adds windows build scripts

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

- C99 ~ C++ compatibility
- Being trivially included, distributed and used
- Self-contained. The library has no external and CRT dependencies.
- Code clarity. The code is sprinkled with holy comments.

Goals/Functions provided by the library:

- Overflow signaling
- Basic arithmetic, Bitwise operators, GCD, LCM

## Current status

- GCD, LCM are not implemented
- Overflow handling is not implemented
- The library has poor support for C++.

## Usage

To use the library download `bn.h`, and include it into every file that
requires the functionality of the library.

```c
#include <bn.h>
```

In **exactly one** file that includes the header, define `bn_implementation`:

```c
#define bn_implementation
#include <bn.h>
```

### Changing the digit size

You can change the "digit" of the underlying representation to be either
1 byte, 2 bytes or 4 bytes. To change the size of the digit, use one of the
following lines

```c
#define bn_word_size 1
#define bn_word_size 2
#define bn_word_size 4
```

In EVERY file that includes `bn.h`. Alternatively add `-Dbn_word_size=n`
compiler option for every compiled file.

### Changing the precision

You can change the max number of digits in a single bigint (in terms of digits)
by defining `bn_array_size`. By default bn_array_size is defined in such a way
as to make the precision of 1024 bits.

If you change the precision, you have to have the same define in EVERY file
that includes `bn.h`. Alternatively add `-Dbn_word_size=n` compiler option
for every compiled file.

### No-CRT builds

To make sure the library doesn't use the CRT you have to:

- `#define bn_assert(expr, msg)` to be either your implementation of assert or
  an empty macro.

## License

All material in this repository is in the public domain.

