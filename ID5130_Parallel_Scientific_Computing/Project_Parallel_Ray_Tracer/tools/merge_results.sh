#!/bin/bash
# Merge the four timings.csv files into a single dataset, dropping failed rows.
#
# Sources:
#   R1: results/20260506_141945/        — original run (OMP + Mitsuba scalar good; MPI bad)
#   R2: results/20260506_200737_rescue/ — first rescue (Mitsuba GT + LLVM good; MPI bad)
#   R3: results/20260507_015203/        — full paper re-run (more OMP data; MPI bad)
#   R4: results/*_mpirescue/            — defensive MPI rescue (MPI good — 90 rows)
#
# Failed rows from R1, R2, R3 are filtered out by exit_code == 0; the working
# subset of each contributes its data. R4 is the only source of valid MPI data.
#
# Output: results/merged/timings.csv  (concatenated, exit_code==0 only)
#         results/merged/summary.csv  (aggregated, median + IQR per config)
#         results/merged/gt_mitsuba_8192spp.exr  (copied from R2)

set -e

R1=results/20260506_141945
R2=results/20260506_200737_rescue
R3=results/20260507_015203
R4=$(ls -td results/*_mpirescue/ | head -1)
OUT=results/merged

[ -f "$R1/timings.csv" ] || { echo "missing $R1/timings.csv"; exit 1; }
[ -f "$R2/timings.csv" ] || { echo "missing $R2/timings.csv"; exit 1; }
[ -f "$R3/timings.csv" ] || { echo "missing $R3/timings.csv"; exit 1; }
[ -f "$R4/timings.csv" ] || { echo "missing $R4/timings.csv"; exit 1; }

mkdir -p "$OUT"
echo "merging:"
echo "  R1 = $R1   ($(wc -l < $R1/timings.csv) lines total)"
echo "  R2 = $R2   ($(wc -l < $R2/timings.csv) lines total)"
echo "  R3 = $R3   ($(wc -l < $R3/timings.csv) lines total)"
echo "  R4 = $R4   ($(wc -l < $R4/timings.csv) lines total)"
echo

# header from R1, then exit_code==0 rows from each
{
    head -1 "$R1/timings.csv"
    awk -F, 'NR > 1 && $14 == "0"' "$R1/timings.csv"
    awk -F, 'NR > 1 && $14 == "0"' "$R2/timings.csv"
    awk -F, 'NR > 1 && $14 == "0"' "$R3/timings.csv"
    awk -F, 'NR > 1 && $14 == "0"' "$R4/timings.csv"
} > "$OUT/timings.csv"

# Copy the ground-truth EXR from R2 (only place it succeeded)
if [ -f "$R2/gt_mitsuba_8192spp.exr" ]; then
    cp -f "$R2/gt_mitsuba_8192spp.exr" "$OUT/"
    echo "GT EXR copied to $OUT/"
fi

N_TOTAL=$(( $(wc -l < $OUT/timings.csv) - 1 ))
echo "merged timings.csv has $N_TOTAL data rows (after dropping exit!=0)"
echo

# breakdown by source so we can sanity-check
echo "rows per renderer in merged set:"
awk -F, 'NR>1 {n[$1]++} END {for (r in n) printf "  %-15s %d\n", r, n[r]}' "$OUT/timings.csv" | sort
echo

echo "aggregating to summary.csv..."
python3 tools/aggregate.py "$OUT/timings.csv" -o "$OUT/summary.csv"
echo
echo "done.  $OUT/timings.csv  +  $OUT/summary.csv"
