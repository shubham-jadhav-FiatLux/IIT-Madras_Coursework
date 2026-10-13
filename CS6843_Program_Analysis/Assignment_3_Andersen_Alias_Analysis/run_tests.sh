#!/usr/bin/env bash

./build.sh

PLUGIN="./build/analysis/a3Pass.so"
# If you're using Mac, instead of .so use .dylib

for src in $(ls testcases/testcase*.c | sort -V); do
    base=$(basename "$src" .c)
    llfile="testcases/$base.ll"
    outfile="testcases/${base}.out"
    expected="testcases/out_${base##testcase}.txt"

    echo "Running testcase: $src"

    clang -S -emit-llvm -g -O0 -w "$src" -o "$llfile"

    opt -load-pass-plugin "$PLUGIN" -passes='pointer-analysis' -disable-output "$llfile" 2> "$outfile"

    if grep -F -x -f "$expected" "$outfile" >/dev/null; then
        echo "✅ $base: PASS"
    else
        echo "❌ $base: FAIL"
        echo "   See diff:"
        diff "$outfile" "$expected"
    fi

    rm -f "$outfile"
done
