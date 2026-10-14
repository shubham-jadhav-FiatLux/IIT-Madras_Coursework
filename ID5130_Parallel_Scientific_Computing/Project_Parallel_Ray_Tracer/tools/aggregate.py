#!/usr/bin/env python3
"""
aggregate.py — group bench.py CSV rows by configuration and emit summary stats.

Reads a long-form CSV (one row per repeat) produced by bench.py and outputs a
wide-form CSV with one row per (renderer, scene, variant, ranks, threads, spp)
group, including:
    n           — number of repeats
    median_ms   — median render_ms (preferred when available, else wall_ms)
    iqr_ms      — Q3 - Q1
    min_ms / max_ms
    mean_ms / std_ms
    md5_match   — True iff all repeats produced same MD5

Use:
    python3 tools/aggregate.py timings.csv -o summary.csv
    python3 tools/aggregate.py timings.csv --by-renderer  # printable table
"""
from __future__ import annotations

import argparse
import csv
import statistics
from collections import defaultdict
from pathlib import Path


GROUP_KEYS = ("renderer", "scene", "variant", "W", "H", "spp", "ranks", "threads")


def parse_float(s: str) -> float | None:
    try:
        v = float(s)
        return v if v == v else None  # NaN check
    except (ValueError, TypeError):
        return None


def aggregate(rows: list[dict]) -> list[dict]:
    by_key: dict[tuple, list[dict]] = defaultdict(list)
    for r in rows:
        by_key[tuple(r[k] for k in GROUP_KEYS)].append(r)

    out = []
    for key, group in sorted(by_key.items()):
        # Prefer renderer-reported render_ms; fall back to wall_ms.
        times = [parse_float(r["render_ms"]) or parse_float(r["wall_ms"])
                 for r in group]
        times = [t for t in times if t is not None]
        if not times:
            continue
        gathers = [parse_float(r.get("gather_ms", "")) for r in group]
        gathers = [g for g in gathers if g is not None]
        md5s = {r["md5"] for r in group if r["md5"]}

        sorted_t = sorted(times)
        n = len(sorted_t)
        q1 = statistics.quantiles(sorted_t, n=4)[0] if n >= 4 else sorted_t[0]
        q3 = statistics.quantiles(sorted_t, n=4)[2] if n >= 4 else sorted_t[-1]

        row = dict(zip(GROUP_KEYS, key))
        row.update({
            "n":         n,
            "median_ms": f"{statistics.median(sorted_t):.3f}",
            "iqr_ms":    f"{q3 - q1:.3f}",
            "min_ms":    f"{sorted_t[0]:.3f}",
            "max_ms":    f"{sorted_t[-1]:.3f}",
            "mean_ms":   f"{statistics.fmean(sorted_t):.3f}",
            "std_ms":    f"{statistics.pstdev(sorted_t):.3f}" if n > 1 else "0",
            "median_gather_ms": (f"{statistics.median(gathers):.3f}"
                                 if gathers else ""),
            "md5_match": "yes" if len(md5s) == 1 else
                         ("no" if len(md5s) > 1 else "n/a"),
            "md5":       next(iter(md5s)) if len(md5s) == 1 else "",
        })
        out.append(row)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("input", type=Path)
    ap.add_argument("-o", "--out", type=Path, default=None,
                    help="Write summary CSV (default: stdout pretty table)")
    ap.add_argument("--by-renderer", action="store_true",
                    help="Group printable output by renderer")
    args = ap.parse_args()

    with open(args.input) as f:
        rows = list(csv.DictReader(f))

    summary = aggregate(rows)

    if args.out:
        if not summary:
            print("no data")
            return
        with open(args.out, "w", newline="") as f:
            w = csv.DictWriter(f, fieldnames=list(summary[0].keys()))
            w.writeheader()
            for r in summary:
                w.writerow(r)
        print(f"wrote {len(summary)} rows to {args.out}")
        return

    # Pretty-print to stdout
    if not summary:
        print("(no data)")
        return
    cols = ["renderer", "scene", "variant", "ranks", "threads", "spp",
            "n", "median_ms", "iqr_ms", "min_ms", "md5_match"]
    widths = {c: max(len(c), max(len(str(r.get(c, ""))) for r in summary))
              for c in cols}
    print(" | ".join(c.ljust(widths[c]) for c in cols))
    print("-+-".join("-" * widths[c] for c in cols))
    for r in summary:
        print(" | ".join(str(r.get(c, "")).ljust(widths[c]) for c in cols))


if __name__ == "__main__":
    main()
