#!/usr/bin/env bash
# Build the simulator and diff each validation / sample run against its reference output (diff -iw).
cd "$(dirname "$0")" || exit 1
make -s || exit 1
pass=0; fail=0
for f in validation/*.txt; do
    cmd=$(sed -n 2p "$f" | sed 's/^ *//')
    eval "$cmd" > /tmp/bpsim_out.txt
    if diff -iw /tmp/bpsim_out.txt "$f" > /dev/null; then pass=$((pass+1)); else echo "FAIL $f"; fail=$((fail+1)); fi
done
echo "passed $pass, failed $fail"; [ "$fail" -eq 0 ]
