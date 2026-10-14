#!/usr/bin/env bash
# Build cs24s009.cu and check every test in testcases/input against testcases/output.
# Usage: ./run_tests.sh
cd "$(dirname "$0")" || exit 1
nvcc -O2 -std=c++17 cs24s009.cu -o cs24s009 || exit 1
pass=0; fail=0
for f in testcases/input/*; do
    n=$(basename "$f")
    ./cs24s009 < "$f" > /dev/null
    if diff -b "testcases/output/$n" cuda.out > /dev/null 2>&1; then echo "$n success"; pass=$((pass+1))
    else echo "$n failure"; fail=$((fail+1)); fi
done
echo "Summary: $pass success, $fail failure"
[ "$fail" -eq 0 ]
