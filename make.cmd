@echo off

if "%1"=="test" (
  set CC=gcc -I. -Wundef -Wall -Wextra -O3
  %CC% bn.c .\tests\golden.c      -o .\build\test_golden
  %CC% bn.c .\tests\hand_picked.c -o .\build\test_hand_picked
  %CC% bn.c .\tests\load_cmp.c    -o .\build\test_load_cmp
  %CC% bn.c .\tests\factorial.c   -o .\build\test_factorial
  %CC% bn.c .\tests\randomized.c  -o .\build\test_random
  REM %CC% bn.c .\tests\rsa.c         -o .\build\test_rsa

  echo.
  echo ======================================================================
  .\build\test_golden
  echo ======================================================================
  .\build\test_hand_picked
  echo ======================================================================
  .\build\test_load_cmp
  echo ======================================================================
  py .\scripts\fact100.py
  .\build\test_factorial
  echo ======================================================================
  py .\scripts\test_old_errors.py
  echo ======================================================================
  REM .\build\test_rsa
  REM echo =======================================================================
  py .\scripts\test_rand.py 1000
  echo ======================================================================
  echo.
) else (

)