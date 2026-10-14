#!/usr/bin/env bash
# Build the simulator and diff the two validation runs against the reference outputs (diff -iw).
cd "$(dirname "$0")" || exit 1
make -s || exit 1
fail=0
./ooosim 8 64 gcc_trace.txt  | diff -iw - validation/gcc_val.txt  > /dev/null && echo "gcc  N=8 S=64: PASS" || { echo "gcc FAIL"; fail=1; }
./ooosim 3 16 perl_trace.txt | diff -iw - validation/perl_val.txt > /dev/null && echo "perl N=3 S=16: PASS" || { echo "perl FAIL"; fail=1; }
exit $fail
