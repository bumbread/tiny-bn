
# tiny-bignum-c - A small multiple-precision integer implementation in C

**WARNING:** Don't use this library for your project. It is not tested and not guaranteed to work. Much of the code is WIP.

This library is a fork of kokke's [tiny-bignum-c](https://github.com/kokke/tiny-bignum-c) library. This fork introduces several changes into the original library:

- It is a single-header library that can be used in C (>= C99) and C++.
- No CRT dependency (except assert macro, that could be disabled)
- Removes UB
- Adds prefixes to the library's functions, defines et cetera
- Adds windows build scripts
- Removes the ability to chose between different digit sizes.

## Description

Tiny Bignum is a small portable [Arbitrary-precision](https://en.wikipedia.org/wiki/Arbitrary-precision_arithmetic)
  unsigned integer arithmetic library, written in C99, for calculating with
  large numbers. The actual precision of the big integer type used in the
  library is unlimited in practice, but in fact it is limited to 128 bytes.
  That limit could be changed by defining a macro.

The library offers no overflow handling for when the integer grows over the
  size of the underlying representation. Thus all operations truncate.

No dynamic memory management is utilized, and the library has no CRT
  dependencies, save for `<assert.h>`'s during debug builds. A custom
  assert macro can be redefined if you wish to not depend on CRT.

## Design goals

- C99 ~ C++ compatibility
- Being trivially included, distributed and used
- Self-contained. The library has no external and CRT dependencies

Functions provided by the library:

- Overflow signaling, as a boolean flag

## Current status

- GCD, LCM, bitwise arithmetic are not implemented
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

### Changing the precision

You can change the max number of digits in a single bigint (in terms of digits)
by defining `bn_array_size`. By default bn_array_size is defined in such a way
as to make the precision of 1024 bits.

If you change the precision, you have to have the same define in EVERY file
that includes `bn.h`. Alternatively add `-Dbn_word_size=n` compiler option
for every compiled file.

### No-CRT builds

To make sure the library doesn't use the CRT you have to:

- `#define bn_assert(expr)` to be either your implementation of assert or
  an empty macro.

## License

All material in this repository is in the public domain.

