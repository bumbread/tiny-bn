@echo off

:: TODO(bumbread): figure out how to mitigate C5045 without disabling
set CC=cl /TC /Z7 /I. /Wall /wd4820 /wd5045

if "%1"=="test" (
  %CC% /LD -Dbn_array_size=4 -Dbn_test -Dbn_implementation bn.h /link /out:tests\tinybignum.dll
  py tests\tinybignum.py
) else (
  %CC% -Dbn_implementation -c bn.h -o build\bignum.obj
)
