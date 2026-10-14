#!/usr/bin/env python3
"""Generate all plots for report.tex from results/merged/summary.csv."""
import csv, os, sys
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SUMMARY = os.path.join(ROOT, "results/merged/summary.csv")
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "figures")
os.makedirs(OUT, exist_ok=True)

# Publication-style defaults
plt.rcParams.update({
    "font.family": "serif",
    "font.size": 9,
    "axes.titlesize": 10,
    "axes.labelsize": 9,
    "legend.fontsize": 8,
    "xtick.labelsize": 8,
    "ytick.labelsize": 8,
    "lines.linewidth": 1.4,
    "lines.markersize": 5,
    "axes.grid": True,
    "grid.alpha": 0.25,
    "grid.linestyle": ":",
    "axes.edgecolor": "#888",
    "axes.linewidth": 0.7,
    "axes.spines.top": False,
    "axes.spines.right": False,
    "savefig.bbox": "tight",
    "savefig.dpi": 300,
})

# Soft, publication-friendly palette
C_PRIMARY = "#5B8DB8"   # soft blue
C_SECOND  = "#E08B5C"   # soft amber
C_THIRD   = "#7FB37F"   # soft green
C_FOURTH  = "#B58FB6"   # soft mauve
C_GREY    = "#9aa0a6"   # neutral grey for ideal/reference lines
C_DARK    = "#33475B"   # near-black for axis text where needed


def load(path=SUMMARY):
    with open(path) as f:
        return list(csv.DictReader(f))


def f(row, key, default=0.0):
    v = row.get(key, "")
    try:
        return float(v) if v else default
    except ValueError:
        return default


def i(row, key, default=0):
    v = row.get(key, "")
    try:
        return int(float(v)) if v else default
    except ValueError:
        return default


def select(rows, **kw):
    out = []
    for r in rows:
        ok = True
        for k, v in kw.items():
            if isinstance(v, (list, tuple)):
                if r.get(k, "") not in [str(x) for x in v]:
                    ok = False; break
            else:
                if r.get(k, "") != str(v):
                    ok = False; break
        if ok:
            out.append(r)
    return out


# ---------------------------------------------------------------------------
# 1. OpenMP strong-scaling (path-trace, Cornell 1024^2 256 SPP)
# ---------------------------------------------------------------------------
def plot_omp_strong():
    rows = load()
    sel = select(rows, renderer="ours_omp",
                 scene="scenes/cornell_box_1024_256spp.scene", variant="standard")
    sel = [r for r in sel if i(r, "ranks") == 1]
    pts = sorted([(i(r, "threads"), f(r, "median_ms")) for r in sel])
    Ts = np.array([t for t, _ in pts], dtype=float)
    ms = np.array([m for _, m in pts], dtype=float)
    base = ms[Ts == 1][0]
    speedup = base / ms
    eff = 100.0 * speedup / Ts

    fig, ax1 = plt.subplots(figsize=(4.0, 2.7))
    ax1.plot(Ts, speedup, "o-", color=C_PRIMARY, label="Measured speed-up")
    ax1.plot([1, Ts.max()], [1, Ts.max()], "--", color=C_GREY,
             linewidth=1.0, label="Ideal (linear)")
    ax1.set_xlabel("OpenMP threads")
    ax1.set_ylabel("Speed-up")
    ax1.set_xscale("log", base=2)
    ax1.set_yscale("log", base=2)
    ax1.set_xticks(Ts); ax1.set_xticklabels([str(int(t)) for t in Ts])
    ax1.set_yticks([1, 2, 4, 8, 16, 32])
    ax1.set_yticklabels(["1", "2", "4", "8", "16", "32"])

    ax2 = ax1.twinx()
    ax2.plot(Ts, eff, "s-", color=C_SECOND, markersize=4, label="Efficiency")
    ax2.set_ylabel("Efficiency (\\%)")
    ax2.set_ylim(0, 110)
    ax2.grid(False)

    lines1, labels1 = ax1.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax1.legend(lines1 + lines2, labels1 + labels2, loc="lower right",
               framealpha=0.9, fontsize=7)
    plt.savefig(os.path.join(OUT, "omp_strong_scaling.pdf"))
    plt.close()


# ---------------------------------------------------------------------------
# 2. MPI strong-scaling (pure MPI, 1 thread/rank)
# ---------------------------------------------------------------------------
def plot_mpi_strong():
    rows = load()
    sel = select(rows, renderer="ours_mpi",
                 scene="scenes/cornell_box_1024_256spp.scene", variant="standard")
    sel = [r for r in sel if i(r, "threads") == 1]
    pts = sorted([(i(r, "ranks"), f(r, "median_ms")) for r in sel])
    Ns = np.array([n for n, _ in pts], dtype=float)
    ms = np.array([m for _, m in pts], dtype=float)
    base = ms[Ns == 1][0]
    speedup = base / ms

    fig, ax = plt.subplots(figsize=(4.0, 2.7))
    ax.plot(Ns, speedup, "o-", color=C_PRIMARY, label="Measured")
    ax.plot([1, Ns.max()], [1, Ns.max()], "--", color=C_GREY,
            linewidth=1.0, label="Ideal")
    ax.set_xlabel("MPI ranks (1 thread per rank)")
    ax.set_ylabel("Speed-up vs np = 1")
    ax.set_xscale("log", base=2)
    ax.set_yscale("log", base=2)
    ax.set_xticks(Ns); ax.set_xticklabels([str(int(n)) for n in Ns],
                                          rotation=0, fontsize=7)
    ax.legend(loc="upper left", framealpha=0.9)
    ax.set_title("Pure-MPI strong scaling")
    plt.savefig(os.path.join(OUT, "mpi_strong_scaling.pdf"))
    plt.close()


# ---------------------------------------------------------------------------
# 3. Adaptive vs Standard bar chart (Whitted showcase, T=40)
# ---------------------------------------------------------------------------
def plot_adaptive_vs_standard():
    rows = load()
    scenes = [
        ("scenes/whitted_showcase_1024.scene", "Whitted\\nshowcase"),
        ("scenes/cornell_box_1024_256spp.scene", "Cornell Box"),
        ("scenes/csg_swiss_cheese.scene", "CSG\\nSwiss-cheese"),
        ("scenes/csg_lattice.scene", "CSG\\nlattice"),
        ("scenes/csg_menger.scene", "CSG\\nMenger"),
    ]
    std_t40, adp_t40 = [], []
    labels = []
    for s, lab in scenes:
        sel = [r for r in select(rows, renderer="ours_omp", scene=s)
               if i(r, "threads") == 40]
        std = next((f(r, "median_ms") for r in sel if r["variant"] == "standard"), None)
        adp = next((f(r, "median_ms") for r in sel if r["variant"] == "adaptive"), None)
        if std is None or adp is None:
            continue
        labels.append(lab.replace("\\n", "\n"))
        std_t40.append(std)
        adp_t40.append(adp)

    x = np.arange(len(labels))
    w = 0.36
    fig, ax = plt.subplots(figsize=(4.6, 2.7))
    ax.bar(x - w/2, std_t40, w, label="Standard", color=C_FOURTH,
           edgecolor="white", linewidth=0.6)
    ax.bar(x + w/2, adp_t40, w, label="Adaptive (LPT)", color=C_PRIMARY,
           edgecolor="white", linewidth=0.6)
    ax.set_xticks(x); ax.set_xticklabels(labels, fontsize=7.5)
    ax.set_yscale("log")
    ax.set_ylabel("Median render time at T=40 (ms, log)")
    ax.legend(framealpha=0.9, loc="upper right")
    for j, (s, a) in enumerate(zip(std_t40, adp_t40)):
        ax.text(j - w/2, s * 1.05, f"{s:.0f}",
                ha="center", va="bottom", fontsize=6.5)
        ax.text(j + w/2, a * 1.05, f"{a:.0f}",
                ha="center", va="bottom", fontsize=6.5)
    plt.savefig(os.path.join(OUT, "adaptive_vs_standard.pdf"))
    plt.close()


# ---------------------------------------------------------------------------
# 4. Whitted showcase strong scaling (std + adaptive)
# ---------------------------------------------------------------------------
def plot_whitted_strong():
    rows = load()
    scene = "scenes/whitted_showcase_1024.scene"
    fig, ax = plt.subplots(figsize=(4.0, 2.7))
    for variant, color, marker in [("standard", C_FOURTH, "o"),
                                     ("adaptive", C_PRIMARY, "s")]:
        sel = select(rows, renderer="ours_omp", scene=scene, variant=variant)
        pts = sorted([(i(r, "threads"), f(r, "median_ms")) for r in sel])
        Ts = np.array([t for t, _ in pts], dtype=float)
        ms = np.array([m for _, m in pts], dtype=float)
        base = ms[Ts == 1][0]
        ax.plot(Ts, base / ms, marker=marker, linestyle="-", color=color,
                label=variant.capitalize())
    ax.plot([1, Ts.max()], [1, Ts.max()], "--", color=C_GREY,
            linewidth=1.0, label="Ideal")
    ax.set_xlabel("OpenMP threads")
    ax.set_ylabel("Speed-up")
    ax.set_xscale("log", base=2); ax.set_yscale("log", base=2)
    ax.set_xticks(Ts); ax.set_xticklabels([str(int(t)) for t in Ts])
    ax.legend(loc="upper left", framealpha=0.9)
    plt.savefig(os.path.join(OUT, "whitted_strong_scaling.pdf"))
    plt.close()


# ---------------------------------------------------------------------------
# 5. Weak scaling
# ---------------------------------------------------------------------------
def plot_weak_scaling():
    rows = load()
    pts = []
    for r in rows:
        sc = r.get("scene", "")
        if "cornell_w_" in sc and r.get("renderer") == "ours_omp":
            n = i(r, "threads"); t = f(r, "median_ms"); res = sc.split("_")[-1].split(".")[0]
            pts.append((n, t, int(res)))
    pts.sort()
    Ns = np.array([n for n, _, _ in pts])
    ms = np.array([t for _, t, _ in pts])
    res = [r for _, _, r in pts]

    fig, ax = plt.subplots(figsize=(4.0, 2.7))
    ax.plot(Ns, ms / 1000.0, "o-", color=C_PRIMARY)
    ax.axhline(ms[0] / 1000.0, ls="--", color=C_GREY, linewidth=1.0,
               label="Ideal (constant time)")
    for n, m, r in zip(Ns, ms, res):
        ax.annotate(f"{r}²", xy=(n, m / 1000.0), xytext=(0, 6),
                    textcoords="offset points", ha="center", fontsize=7)
    ax.set_xscale("log", base=2)
    ax.set_xticks(Ns); ax.set_xticklabels([str(int(n)) for n in Ns])
    ax.set_xlabel("N threads (image area $\\propto N$)")
    ax.set_ylabel("Median time (s)")
    ax.legend(framealpha=0.9, loc="upper left")
    plt.savefig(os.path.join(OUT, "weak_scaling.pdf"))
    plt.close()


# ---------------------------------------------------------------------------
# 6. Time-to-quality vs Mitsuba (throughput-normalised)
# ---------------------------------------------------------------------------
def plot_ttq():
    rows = load()
    fig, ax = plt.subplots(figsize=(4.4, 2.9))

    # Ours (T=40, 1024^2)
    ours = []
    for r in rows:
        sc = r.get("scene", "")
        if "cornell_1024_spp" in sc and r.get("renderer") == "ours_omp":
            spp = int(sc.split("spp")[-1].split(".")[0])
            ours.append((spp, f(r, "median_ms") / 1000.0))
    ours.sort()
    if ours:
        ax.plot([s for s, _ in ours], [t for _, t in ours],
                "o-", color=C_PRIMARY, label="Ours OpenMP, T=40 (1024²)")

    # Mitsuba LLVM T=40
    mll = sorted([(i(r, "spp"), f(r, "median_ms") / 1000.0)
                  for r in select(rows, renderer="mitsuba_llvm")
                  if i(r, "spp") <= 1024])
    if mll:
        ax.plot([s for s, _ in mll], [t for _, t in mll],
                "s-", color=C_THIRD, label="Mitsuba 3 LLVM, T=40 (512²)")

    # Mitsuba scalar T=1
    msc = sorted([(i(r, "spp"), f(r, "median_ms") / 1000.0)
                  for r in select(rows, renderer="mitsuba_scalar")
                  if i(r, "spp") <= 1024])
    if msc:
        ax.plot([s for s, _ in msc], [t for _, t in msc],
                "^-", color=C_SECOND, label="Mitsuba 3 scalar, T=1 (512²)")

    ax.set_xscale("log", base=2); ax.set_yscale("log", base=10)
    ax.set_xlabel("Samples per pixel")
    ax.set_ylabel("Wall-clock render time (s)")
    ax.legend(loc="upper left", framealpha=0.9, fontsize=7)
    plt.savefig(os.path.join(OUT, "time_to_quality.pdf"))
    plt.close()


# ---------------------------------------------------------------------------
# 7. CSG scenes scaling
# ---------------------------------------------------------------------------
def plot_csg_scaling():
    rows = load()
    fig, ax = plt.subplots(figsize=(4.0, 2.7))
    scenes = [
        ("scenes/csg_swiss_cheese.scene", "Swiss-cheese", "o", C_PRIMARY),
        ("scenes/csg_lattice.scene", "Lattice", "s", C_THIRD),
        ("scenes/csg_menger.scene", "Menger", "^", C_SECOND),
    ]
    for s, lab, mk, col in scenes:
        sel = select(rows, renderer="ours_omp", scene=s, variant="standard")
        pts = sorted([(i(r, "threads"), f(r, "median_ms")) for r in sel])
        if not pts:
            continue
        Ts = np.array([t for t, _ in pts], dtype=float)
        ms = np.array([m for _, m in pts], dtype=float)
        ax.plot(Ts, ms[0] / ms, marker=mk, linestyle="-", color=col, label=lab)
    Tref = np.array([1, 8, 40], dtype=float)
    ax.plot(Tref, Tref, "--", color=C_GREY, linewidth=1.0, label="Ideal")
    ax.set_xscale("log", base=2); ax.set_yscale("log", base=2)
    ax.set_xticks([1, 8, 40]); ax.set_xticklabels(["1", "8", "40"])
    ax.set_yticks([1, 2, 4, 8, 16, 32])
    ax.set_yticklabels(["1", "2", "4", "8", "16", "32"])
    ax.set_xlabel("OpenMP threads")
    ax.set_ylabel("Speed-up vs T=1")
    ax.legend(loc="upper left", framealpha=0.9, fontsize=7)
    plt.savefig(os.path.join(OUT, "csg_scaling.pdf"))
    plt.close()


if __name__ == "__main__":
    plot_omp_strong()
    plot_mpi_strong()
    plot_adaptive_vs_standard()
    plot_whitted_strong()
    plot_weak_scaling()
    plot_csg_scaling()
    print("All plots written to", OUT)
