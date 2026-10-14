#!/usr/bin/env python3
"""
Parse AQUA benchmark log files and print formatted tables.

Usage:
    python3 cluster/collect_results.py rt_omp_scaling.log
    python3 cluster/collect_results.py rt_mpi_scaling.log
    python3 cluster/collect_results.py rt_omp_scaling.log rt_mpi_scaling.log
"""
import sys, re
from collections import defaultdict
from statistics import median

def parse_kv(line):
    """Parse 'KEY=val KEY=val ...' into a dict."""
    return dict(re.findall(r'(\w+)=([\d.]+)', line))

def best(values):
    return min(float(v) for v in values)

def speedup(base, t):
    return base / t

def efficiency(base, t, n):
    return speedup(base, t) / n * 100

# ── collect raw data ─────────────────────────────────────────────────────────
data = defaultdict(list)   # key -> list of ms values

for path in sys.argv[1:]:
    try:
        lines = open(path).readlines()
    except FileNotFoundError:
        print(f"File not found: {path}", file=sys.stderr)
        continue
    for line in lines:
        line = line.strip()
        if not line or line.startswith('=') or line.startswith('#'):
            continue
        kv = parse_kv(line)
        if 'ms' not in kv:
            continue
        tag = line.split()[0]
        data[(tag, line)].append(float(kv['ms']))

# group by (tag, config-without-R)
grouped = defaultdict(list)
for (tag, line), vals in data.items():
    # drop R= from key
    key = re.sub(r'\s*R=\d+', '', line).strip()
    grouped[key].extend(vals)

# ── print tables ─────────────────────────────────────────────────────────────

def print_table(title, rows, headers):
    print(f"\n{'='*60}")
    print(f"  {title}")
    print(f"{'='*60}")
    widths = [max(len(str(r[i])) for r in rows + [headers]) for i in range(len(headers))]
    fmt = '  '.join(f'{{:<{w}}}' for w in widths)
    print(fmt.format(*headers))
    print('  '.join('-'*w for w in widths))
    for row in rows:
        print(fmt.format(*row))

# ── Whitted OMP Standard ──────────────────────────────────────────────────────
rows = []
baseline = None
for key in sorted(grouped):
    if 'WHITTED_STD' not in key:
        continue
    kv = parse_kv(key)
    t = int(kv.get('T', 0))
    ms = best(grouped[key])
    if t == 1:
        baseline = ms
    sp = f"{speedup(baseline, ms):.2f}x" if baseline else "—"
    eff = f"{efficiency(baseline, ms, t):.1f}%" if baseline else "—"
    rows.append((t, f"{ms:.1f}", sp, eff))
if rows:
    print_table("Whitted — Standard (OMP scaling)", rows,
                ("Threads", "Best ms", "Speedup", "Efficiency"))

# ── Whitted OMP Adaptive ──────────────────────────────────────────────────────
rows = []
baseline = None
for key in sorted(grouped):
    if 'WHITTED_ADAPTIVE' not in key:
        continue
    kv = parse_kv(key)
    t = int(kv.get('T', 0))
    ms = best(grouped[key])
    if t == 1:
        baseline = ms
    sp = f"{speedup(baseline, ms):.2f}x" if baseline else "—"
    eff = f"{efficiency(baseline, ms, t):.1f}%" if baseline else "—"
    rows.append((t, f"{ms:.1f}", sp, eff))
if rows:
    print_table("Whitted — Adaptive LPT (OMP scaling)", rows,
                ("Threads", "Best ms", "Speedup", "Efficiency"))

# ── Path Trace 256 OMP ───────────────────────────────────────────────────────
rows = []
baseline = None
for key in sorted(grouped):
    if 'PT256' not in key or 'HYBRID' in key or 'MPI' in key:
        continue
    kv = parse_kv(key)
    t = int(kv.get('T', 0))
    ms = best(grouped[key])
    sps = 512*512*256 / ms / 1000
    if t == 1:
        baseline = ms
    sp = f"{speedup(baseline, ms):.2f}x" if baseline else "—"
    eff = f"{efficiency(baseline, ms, t):.1f}%" if baseline else "—"
    rows.append((t, f"{ms:.0f}", f"{sps:.2f}", sp, eff))
if rows:
    print_table("Path Trace 256 SPP — OMP scaling", rows,
                ("Threads", "Best ms", "MSamples/s", "Speedup", "Efficiency"))

# ── Path Trace 256 Hybrid ────────────────────────────────────────────────────
rows = []
baseline = None
for key in sorted(grouped, key=lambda k: int(parse_kv(k).get('NP', 0) or 0)*1000 +
                                          int(parse_kv(k).get('OT', 0) or 0)):
    if 'PT256_HYBRID' not in key:
        continue
    kv = parse_kv(key)
    np_ = int(kv.get('NP', 0))
    ot  = int(kv.get('OT', 0))
    total = int(kv.get('TOTAL', np_*ot))
    ms = best(grouped[key])
    sps = 512*512*256 / ms / 1000
    if np_ == 1 and ot == total:
        baseline = ms
    sp  = f"{speedup(baseline, ms):.2f}x" if baseline else "—"
    eff = f"{efficiency(baseline, ms, total):.1f}%" if baseline else "—"
    rows.append((np_, ot, total, f"{ms:.0f}", f"{sps:.2f}", sp, eff))
if rows:
    print_table("Path Trace 256 SPP — Hybrid MPI+OMP", rows,
                ("MPI ranks", "OMP t", "Total", "Best ms", "MSamples/s", "Speedup", "Efficiency"))

# ── Whitted Pure MPI ──────────────────────────────────────────────────────────
rows = []
baseline = None
for key in sorted(grouped, key=lambda k: int(parse_kv(k).get('NP', 0) or 0)):
    if 'WHITTED_PURE_MPI' not in key:
        continue
    kv = parse_kv(key)
    np_ = int(kv.get('NP', 0))
    ms = best(grouped[key])
    if np_ == 1:
        baseline = ms
    sp = f"{speedup(baseline, ms):.2f}x" if baseline else "—"
    eff = f"{efficiency(baseline, ms, np_):.1f}%" if baseline else "—"
    rows.append((np_, f"{ms:.1f}", sp, eff))
if rows:
    print_table("Whitted — Pure MPI (1 thread/rank)", rows,
                ("MPI ranks", "Best ms", "Speedup", "Efficiency"))

# ── Adaptive vs Standard ──────────────────────────────────────────────────────
std_ms = adp_ms = None
for key in grouped:
    if 'STANDARD' in key:
        std_ms = best(grouped[key])
    if 'ADAPTIVE' in key and 'WHITTED' not in key:
        adp_ms = best(grouped[key])
if std_ms and adp_ms:
    gain = (std_ms - adp_ms) / std_ms * 100
    print(f"\n{'='*60}")
    print(f"  Adaptive LPT vs Standard (MPI, best config)")
    print(f"{'='*60}")
    print(f"  Standard:  {std_ms:.1f} ms")
    print(f"  Adaptive:  {adp_ms:.1f} ms")
    print(f"  Gain:      {gain:.1f}%")

# ── Correctness summary ───────────────────────────────────────────────────────
print(f"\n{'='*60}")
print("  Correctness (from log)")
print(f"{'='*60}")
for path in sys.argv[1:]:
    try:
        for line in open(path):
            if 'OK' in line or 'MISMATCH' in line:
                print(' ', line.strip())
    except FileNotFoundError:
        pass

print()
