#!/bin/bash
# Health-check the latest run results dir (full job_paper.pbs OR merged set).
#
# Usage:
#   bash tools/verify_run.sh                  # auto-pick newest results/<TS>/
#   bash tools/verify_run.sh results/merged   # explicit dir

set -u

RES="${1:-}"
if [ -z "$RES" ]; then
    RES=$(ls -td results/*/ 2>/dev/null | grep -vE '_rescue/|_mpirescue/' | head -1)
fi
RES="${RES%/}"
[ -d "$RES" ] || { echo "no results dir found: $RES"; exit 1; }
echo "Checking: $RES"
echo

CSV="$RES/timings.csv"
LOG="$RES/run.log"
[ -f "$CSV" ] || { echo "FAIL: $CSV missing"; exit 1; }

# (a) row count + tier markers
echo "=== (a) rows + tier markers ==="
TOTAL=$(awk 'NR>1' "$CSV" | wc -l | tr -d ' ')
echo "  total data rows: $TOTAL"
[ -f "$LOG" ] && grep -E "^=== TIER" "$LOG"
echo

# (b) failed runs
echo "=== (b) failed bench runs (exit_code != 0) ==="
N_FAIL=$(awk -F, 'NR>1 && $14 != "0"' "$CSV" | wc -l | tr -d ' ')
echo "  failed rows: $N_FAIL"
[ "$N_FAIL" -gt 0 ] && \
    awk -F, 'NR>1 && $14 != "0" { print "    " $1, $2, "r="$7, "t="$8, "spp="$6, "exit="$14 }' "$CSV" | head
echo

# (c) Whitted bit-identity (one MD5 per Whitted/CSG scene across all configs)
echo "=== (c) Whitted/CSG bit-identity (one MD5 per scene across configs) ==="
for SCENE_PAT in "whitted_showcase" "csg_swiss" "csg_lattice" "csg_menger"; do
    HASHES=$(awk -F, -v p="$SCENE_PAT" \
        'NR>1 && ($1=="ours_omp" || $1=="ours_mpi") && $2 ~ p && $13 != "" {print $13}' \
        "$CSV" | sort -u | wc -l | tr -d ' ')
    [ "$HASHES" = "0" ] && continue
    if [ "$HASHES" -eq 1 ]; then
        echo "  PASS  $SCENE_PAT  (1 hash across all configs)"
    else
        echo "  FAIL  $SCENE_PAT  ($HASHES distinct hashes — bit-identity broken)"
    fi
done
echo

# (d) PT determinism (one MD5 per (scene, spp))
echo "=== (d) Path-trace determinism (one MD5 per (scene,spp)) ==="
N_PT_BAD=$(awk -F, 'NR>1 && ($1=="ours_omp" || $1=="ours_mpi") && $2 ~ /cornell/ && $13 != "" {print $2"|"$6, $13}' "$CSV" \
    | sort -u | awk '{print $1}' | sort | uniq -c | awk '$1>1' | wc -l | tr -d ' ')
N_PT_OK=$(awk -F, 'NR>1 && ($1=="ours_omp" || $1=="ours_mpi") && $2 ~ /cornell/ && $13 != "" {print $2"|"$6, $13}' "$CSV" \
    | sort -u | awk '{print $1}' | sort | uniq -c | awk '$1==1' | wc -l | tr -d ' ')
if [ "$N_PT_BAD" -eq 0 ]; then
    echo "  PASS  $N_PT_OK (scene,spp) tuples each have exactly 1 hash"
else
    echo "  FAIL  $N_PT_BAD (scene,spp) tuples have multiple hashes"
fi
echo

# (e) GT EXR
echo "=== (e) Mitsuba 8192-spp ground truth ==="
GT="$RES/gt_mitsuba_8192spp.exr"
if [ -f "$GT" ]; then
    SZ=$(stat -f %z "$GT" 2>/dev/null || stat -c %s "$GT")
    echo "  PASS  $GT  (${SZ} bytes)"
else
    echo "  WARN  $GT  not found"
fi
echo

# (f) renderer coverage
echo "=== (f) renderer coverage ==="
awk -F, 'NR>1 && $14=="0" {n[$1]++} END { for (r in n) printf "  %-15s %d successful rows\n", r, n[r] }' "$CSV" | sort
echo

# (g) MPI rank coverage (successful only)
echo "=== (g) ours_mpi rank/thread coverage (successful runs) ==="
awk -F, 'NR>1 && $1=="ours_mpi" && $14=="0" {print "r="$7" t="$8}' "$CSV" | sort | uniq -c
echo

# (h) speedup audit
echo "=== (h) OMP speedup audit (cornell_box_1024_256spp.scene) ==="
python3 - "$CSV" <<'PY'
import csv, sys, statistics
path = sys.argv[1]
rows = []
with open(path) as f:
    r = csv.DictReader(f)
    for row in r:
        if row["renderer"] == "ours_omp" and "cornell_box_1024_256spp" in row["scene"] \
                and row["exit_code"] == "0" and row["variant"] == "standard":
            try:    rows.append((int(row["threads"]), float(row["render_ms"])))
            except: pass
by_t = {}
for t, ms in rows: by_t.setdefault(t, []).append(ms)
medians = {t: statistics.median(v) for t, v in by_t.items()}
if 1 in medians:
    base = medians[1]
    for t in sorted(medians):
        sp = base / medians[t]
        eff = sp / t * 100.0
        flag = " <-- SUPER-LINEAR!" if sp > t * 1.05 else ""
        print(f"  T={t:3d}  {medians[t]:9.0f} ms   speedup={sp:5.2f}x   eff={eff:5.1f}%{flag}")
PY
echo
echo "=== verify done ==="
