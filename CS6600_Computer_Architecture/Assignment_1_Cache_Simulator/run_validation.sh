#!/usr/bin/env bash
# Build the simulator and diff it against the reference outputs (diff -iw, as the TAs do).
cd "$(dirname "$0")" || exit 1
make -s || exit 1
pass=0; fail=0
for f in validation/reference/*.txt validation/additional/*.txt; do
    cfg=$(sed -n 2,8p "$f" | tr -d '\r' | awk '{printf "%s ", $2}')
    # shellcheck disable=SC2086
    ./cache_sim $cfg > /tmp/cache_sim_out.txt
    # the additional outputs stop after the average access time (no EDP / area lines)
    if diff -iw <(grep -v -e 'energy-delay product' -e 'total area' /tmp/cache_sim_out.txt) <(grep -v -e 'energy-delay product' -e 'total area' "$f") > /dev/null \
       && { [[ $f == validation/additional/* ]] || diff -iw /tmp/cache_sim_out.txt "$f" > /dev/null; }; then
        pass=$((pass+1)); else echo "FAIL $f"; fail=$((fail+1)); fi
done
echo "passed $pass, failed $fail"; [ "$fail" -eq 0 ]
