#!/bin/bash
# preflight.sh — verify all prerequisites are in place before submitting
# job_paper.pbs. Run this on the AQUA login node *after* setup_pbrt.sh and
# setup_mitsuba.sh, *before* `qsub cluster/job_paper.pbs`.
#
# Exits 0 on green, non-zero on any failure.

set -uo pipefail

# Self-sufficient: source the AQUA env so preflight reflects what the PBS
# job will actually see, not whatever the user's interactive shell has.
# shellcheck disable=SC1091
[ -f cluster/aqua_env.sh ] && source cluster/aqua_env.sh

PASS=0
FAIL=0
WARN=0
say() { printf "%-50s %s\n" "$1" "$2"; }
ok()   { say "$1" "[ok]";   PASS=$((PASS+1)); }
fail() { say "$1" "[FAIL: $2]"; FAIL=$((FAIL+1)); }
warn() { say "$1" "[warn: $2]"; WARN=$((WARN+1)); }

echo "==============================================="
echo " AQUA pre-flight check  $(date)"
echo "==============================================="

# 1. Build status
[ -x build/release/rt ]      && ok "OMP binary exists"      || fail "OMP binary exists"     "run cluster/build.sh"
[ -x build/release/rt_mpi ]  && ok "MPI binary exists"      || fail "MPI binary exists"     "run cluster/build.sh"

# 2. Tests pass
make test MODE=release > /tmp/_pf_tests.log 2>&1 \
    && ok "make test passes"                                 \
    || { tail -5 /tmp/_pf_tests.log; fail "make test passes" "see /tmp/_pf_tests.log"; }

# 3. Scenes resolve
for s in cornell_box_1024.scene cornell_box.xml cornell_box_1024_pbrt.pbrt \
         whitted_showcase.scene csg_swiss_cheese.scene csg_lattice.scene \
         csg_menger.scene csg_deep.scene; do
    [ -f "scenes/$s" ] && ok "scene  $s" || fail "scene  $s" "missing"
done

# 4. Tools resolve
for t in tools/bench.py tools/aggregate.py tools/diff_ppm.py; do
    [ -x "$t" -o -r "$t" ] && ok "tool   $t" || fail "tool   $t" "missing"
done

# 5. External renderers (these are what most users miss)
if command -v pbrt > /dev/null 2>&1; then
    ok "pbrt-v4 on PATH"
    pbrt --help > /dev/null 2>&1 && ok "pbrt --help works" \
                                  || fail "pbrt --help works" "binary present but broken"
else
    # AQUA: pbrt-v4 build is fragile (upstream submodule SHAs gone, libdeflate
    # cmake-config mismatch with ptex). The benchmark gracefully skips pbrt
    # rows; the CSG-uniqueness claim is independent of pbrt.
    warn "pbrt-v4 on PATH" "Tier 3 pbrt rows will be skipped (this is acceptable — see REPORT_CONTEXT.md)"
fi

if command -v mitsuba > /dev/null 2>&1; then
    ok "mitsuba on PATH"
    # Mitsuba 3 has no --version flag; probe via Python import (matches actual usage).
    python3 -c "import mitsuba" 2>/dev/null \
        && ok "mitsuba scalar import works" \
        || fail "mitsuba scalar import works" "venv broken; rerun cluster/setup_mitsuba.sh"
    # LLVM variant requires DRJIT_LIBLLVM_PATH (set by cluster/aqua_env.sh).
    python3 -c "import mitsuba as mi; mi.set_variant('llvm_ad_rgb')" 2>/dev/null \
        && ok "mitsuba LLVM variant works" \
        || warn "mitsuba LLVM variant works" "DRJIT_LIBLLVM_PATH unset/wrong; only scalar will be used in Tier 3"
else
    warn "mitsuba on PATH" "Tier 3 will skip Mitsuba comparison; run cluster/setup_mitsuba.sh"
fi

# 6. Python deps for harness
python3 -c "import numpy" 2>/dev/null && ok "python3 numpy"     || fail "python3 numpy"     "pip install --user numpy"
python3 -c "import imageio" 2>/dev/null && ok "python3 imageio" || warn "python3 imageio"   "pip install --user imageio (only needed for diff_ppm)"
python3 -c "import skimage" 2>/dev/null && ok "python3 skimage" || warn "python3 skimage"   "pip install --user scikit-image (only for SSIM)"

# 7. Disk quota — need ~1 GB for a full benchmark
QUOTA_USED=$(df -P "$HOME" | awk 'NR==2 {print $3}')
QUOTA_AVAIL=$(df -P "$HOME" | awk 'NR==2 {print $4}')
if [ "$QUOTA_AVAIL" -gt 1000000 ]; then
    ok "home dir disk space (>= 1 GB free: $((QUOTA_AVAIL/1024)) MB)"
else
    fail "home dir disk space" "only $((QUOTA_AVAIL/1024)) MB free; benchmark needs ~1 GB for outputs"
fi

# 8. PBS submission ready
if grep -q '^#PBS -l place=excl' cluster/job_paper.pbs; then
    ok "job_paper.pbs requests exclusive nodes"
else
    fail "job_paper.pbs requests exclusive nodes" "place=excl missing"
fi

EMAIL=$(grep -E '^#PBS -M ' cluster/job_paper.pbs | awk '{print $3}')
if [ -n "$EMAIL" ]; then
    ok "PBS email notification: $EMAIL"
else
    warn "PBS email notification" "no -M directive; you'll have to poll qstat"
fi

# Result
echo "-----------------------------------------------"
echo "PASS=$PASS  WARN=$WARN  FAIL=$FAIL"
echo "-----------------------------------------------"
if [ "$FAIL" -gt 0 ]; then
    echo "❌ Pre-flight failed — fix the above before submitting."
    exit 1
fi
if [ "$WARN" -gt 0 ]; then
    echo "⚠️  Warnings present — Tier 3 may produce partial results."
    echo "   You can still submit if you're OK with that."
fi
echo "✅ Pre-flight green. Submit with:"
echo "    qsub cluster/job_paper.pbs"
