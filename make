#!/bin/sh

mv bn.h bn.c

gcc -I . -g -Wall -shared \
    -Dbn_array_size=4 \
	-Dbn_implementation \
	bn.c -otests/tinybignum.so

mv bn.c bn.h

