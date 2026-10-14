#!/usr/bin/env python3
"""
bench.py — single-config benchmark runner.

Runs one renderer + scene + thread/rank configuration N times, parses timings
from stdout, and writes one CSV row per repeat. Intended as the unit-of-work
inside `cluster/job_paper.pbs` and friends.

Usage:
    bench.py --renderer ours_omp     --scene scenes/cornell_box_1024.scene \
             --threads 40 --repeats 5 --out timings.csv
    bench.py --renderer ours_mpi     --scene scenes/cornell_box_1024.scene \
             --ranks 8 --threads 5 --repeats 5 --out timings.csv
    bench.py --renderer pbrt         --scene scenes/cornell_box_1024_pbrt.pbrt \
             --threads 40 --spp 256 --repeats 3 --out timings.csv
    bench.py --renderer mitsuba_llvm --scene scenes/cornell_box.xml \
             --threads 40 --spp 256 --repeats 3 --out timings.csv

CSV columns:
    renderer, scene, variant, W, H, spp, ranks, threads, run_idx,
    wall_ms, render_ms, gather_ms, md5, exit_code, hostname, timestamp

Conventions:
    `wall_ms`   — total wall time of the renderer process
    `render_ms` — what the renderer's own timer reports (compute only)
    `gather_ms` — MPI gather time (only meaningful for ours_mpi); else NaN

Per-row rows are written eagerly (flushed) so a long benchmark job doesn't
lose data on signal/timeout.
"""
from __future__ import annotations

import argparse
import csv
import datetime as dt
import hashlib
import os
import re
import socket
import subprocess
import sys
import time
from pathlib import Path
from typing import Optional


# -------- Renderer-specific argument builders + output parsers ---------------

def _md5(path: Path) -> str:
    if not path.exists():
        return ""
    h = hashlib.md5()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 16), b""):
            h.update(chunk)
    return h.hexdigest()


def cmd_ours_omp(args, scene: Path, output: Path) -> tuple[list[str], dict]:
    """Build env+argv for our OpenMP binary."""
    env = os.environ.copy()
    env["OMP_NUM_THREADS"] = str(args.threads)
    env.setdefault("OMP_PLACES", "cores")
    env.setdefault("OMP_PROC_BIND", "close")
    bin_path = args.binary or "build/release/rt"
    cmd = [bin_path, str(scene), str(output)]
    if args.adaptive:
        cmd.append("--adaptive")
    return cmd, env


def cmd_ours_mpi(args, scene: Path, output: Path) -> tuple[list[str], dict]:
    """Hybrid MPI + OMP.

    Open MPI launches workers with a cwd that may differ from the launcher
    (compute-node-specific behavior we observed on AQUA). Pass --wdir to
    pin the worker cwd, and resolve scene/output to absolute paths so the
    workers can find them regardless of where they actually start.
    """
    env = os.environ.copy()
    env["OMP_NUM_THREADS"] = str(args.threads)
    env.setdefault("OMP_PLACES", "cores")
    env.setdefault("OMP_PROC_BIND", "close")
    bin_path = args.binary or "build/release/rt_mpi"
    abs_bin    = str(Path(bin_path).resolve())
    abs_scene  = str(scene.resolve())
    abs_output = str(output.resolve())
    cwd        = str(Path.cwd().resolve())
    cmd = ["mpirun", "--wdir", cwd]
    if args.mpi_args:
        cmd += args.mpi_args.split()
    cmd += ["-np", str(args.ranks), abs_bin, abs_scene, abs_output]
    if args.adaptive:
        cmd.append("--adaptive")
    return cmd, env


def cmd_pbrt(args, scene: Path, output: Path) -> tuple[list[str], dict]:
    env = os.environ.copy()
    bin_path = args.binary or "pbrt"
    cmd = [bin_path, "--nthreads", str(args.threads),
           "--outfile", str(output), str(scene)]
    if args.spp:
        cmd += ["--spp", str(args.spp)]
    return cmd, env


def cmd_mitsuba(args, scene: Path, output: Path,
                variant: str) -> tuple[list[str], dict]:
    env = os.environ.copy()
    env.setdefault("DRJIT_LLVM_NUM_THREADS", str(args.threads))
    bin_path = args.binary or "mitsuba"
    # Mitsuba 3 CLI: scene must be the LAST positional argument.
    # Options (-m, -o, -t, -D KEY=VAL) come first, scene last.
    cmd = [bin_path, "-m", variant, "-o", str(output)]
    if args.threads:
        cmd += ["-t", str(args.threads)]
    if args.spp:
        cmd += ["-D", f"spp={args.spp}"]
    cmd.append(str(scene))
    return cmd, env


# Matches the line our renderer prints: "... render 1234.5 ms" or
# "... render 1234.5 gather 7.8 ms"
_RE_RENDER = re.compile(r"render\s+([0-9.]+)")
_RE_GATHER = re.compile(r"gather\s+([0-9.]+)")
# Mitsuba prints "Rendering finished. (took 1m23s.)" — convert to ms.
_RE_MI_TOOK = re.compile(r"took\s+(?:(\d+)m)?(\d+(?:\.\d+)?)s")
# pbrt prints "Rendering: 12.3s" or similar after timestamps.
_RE_PBRT_RENDER = re.compile(r"Rendering:\s+([0-9.]+)s")


def parse_timings(stdout: str, renderer: str) -> tuple[float, float]:
    """Returns (render_ms, gather_ms). Either may be NaN if absent."""
    nan = float("nan")
    m_r = _RE_RENDER.search(stdout)
    m_g = _RE_GATHER.search(stdout)
    if m_r:
        return float(m_r.group(1)), (float(m_g.group(1)) if m_g else nan)

    if renderer.startswith("mitsuba"):
        m = _RE_MI_TOOK.search(stdout)
        if m:
            mins = int(m.group(1) or 0)
            secs = float(m.group(2))
            return (mins * 60 + secs) * 1000.0, nan
    if renderer == "pbrt":
        m = _RE_PBRT_RENDER.search(stdout)
        if m:
            return float(m.group(1)) * 1000.0, nan
    return nan, nan


def detect_image_size(scene: Path) -> tuple[int, int]:
    """Best-effort scrape of resolution from a scene file. Used for sps calc."""
    try:
        text = scene.read_text(errors="ignore")
        m = re.search(r"^image\s+(\d+)\s+(\d+)", text, re.MULTILINE)
        if m:
            return int(m.group(1)), int(m.group(2))
        m = re.search(r'xresolution"\s*\[\s*(\d+)\s*\]', text)
        n = re.search(r'yresolution"\s*\[\s*(\d+)\s*\]', text)
        if m and n:
            return int(m.group(1)), int(n.group(1))
        m = re.search(r'name="width"\s+value="(\d+)"', text)
        n = re.search(r'name="height"\s+value="(\d+)"', text)
        if m and n:
            return int(m.group(1)), int(n.group(1))
    except OSError:
        pass
    return 0, 0


# -------- Main runner --------------------------------------------------------

CSV_FIELDS = [
    "renderer", "scene", "variant", "W", "H", "spp", "ranks", "threads",
    "run_idx", "wall_ms", "render_ms", "gather_ms", "md5",
    "exit_code", "hostname", "timestamp",
]


def main():
    ap = argparse.ArgumentParser(description="Single-config benchmark runner")
    ap.add_argument("--renderer", required=True,
                    choices=["ours_omp", "ours_mpi", "pbrt",
                             "mitsuba_scalar", "mitsuba_llvm"])
    ap.add_argument("--scene", required=True, type=Path)
    ap.add_argument("--out", required=True, type=Path,
                    help="CSV file (appended)")
    ap.add_argument("--threads", type=int, default=1)
    ap.add_argument("--ranks", type=int, default=1, help="MPI ranks")
    ap.add_argument("--spp", type=int, default=0,
                    help="0 = scene default; otherwise override")
    ap.add_argument("--repeats", type=int, default=1)
    ap.add_argument("--adaptive", action="store_true")
    ap.add_argument("--binary", default=None,
                    help="override path to renderer binary")
    ap.add_argument("--mpi-args", default=None,
                    help="extra args injected before -np (e.g. '--bind-to core')")
    ap.add_argument("--output-dir", type=Path, default=Path("output/_bench"),
                    help="where rendered images land")
    args = ap.parse_args()

    args.output_dir.mkdir(parents=True, exist_ok=True)
    W, H = detect_image_size(args.scene)

    out_csv = args.out
    new_file = not out_csv.exists()
    f = open(out_csv, "a", newline="")
    w = csv.DictWriter(f, fieldnames=CSV_FIELDS)
    if new_file:
        w.writeheader()
        f.flush()

    for run_idx in range(args.repeats):
        out_img = (args.output_dir /
                   f"{args.renderer}_{args.scene.stem}_t{args.threads}_r{args.ranks}_run{run_idx}"
                   f"{'_adp' if args.adaptive else ''}.{'exr' if 'mitsuba' in args.renderer or args.renderer == 'pbrt' else 'ppm'}")
        if args.renderer == "ours_omp":
            cmd, env = cmd_ours_omp(args, args.scene, out_img)
            variant = "adaptive" if args.adaptive else "standard"
        elif args.renderer == "ours_mpi":
            cmd, env = cmd_ours_mpi(args, args.scene, out_img)
            variant = "adaptive" if args.adaptive else "standard"
        elif args.renderer == "pbrt":
            cmd, env = cmd_pbrt(args, args.scene, out_img)
            variant = "z-sobol"
        elif args.renderer == "mitsuba_scalar":
            cmd, env = cmd_mitsuba(args, args.scene, out_img, "scalar_rgb")
            variant = "scalar_rgb"
        elif args.renderer == "mitsuba_llvm":
            cmd, env = cmd_mitsuba(args, args.scene, out_img, "llvm_ad_rgb")
            variant = "llvm_ad_rgb"
        else:
            raise SystemExit(f"unknown renderer {args.renderer}")

        sys.stderr.write(f"[bench] {' '.join(cmd)}\n")
        t0 = time.monotonic()
        proc = subprocess.run(cmd, env=env, capture_output=True, text=True)
        wall_ms = (time.monotonic() - t0) * 1000.0
        render_ms, gather_ms = parse_timings(proc.stdout + proc.stderr,
                                             args.renderer)
        row = {
            "renderer":   args.renderer,
            "scene":      str(args.scene),
            "variant":    variant,
            "W":          W,
            "H":          H,
            "spp":        args.spp,
            "ranks":      args.ranks,
            "threads":    args.threads,
            "run_idx":    run_idx,
            "wall_ms":    f"{wall_ms:.3f}",
            "render_ms":  f"{render_ms:.3f}" if render_ms == render_ms else "",
            "gather_ms":  f"{gather_ms:.3f}" if gather_ms == gather_ms else "",
            "md5":        _md5(out_img),
            "exit_code":  proc.returncode,
            "hostname":   socket.gethostname(),
            "timestamp":  dt.datetime.now(dt.timezone.utc).isoformat(timespec="seconds"),
        }
        w.writerow(row)
        f.flush()
        sys.stderr.write(f"[bench]   wall={wall_ms:.0f} render={render_ms:.0f} "
                         f"exit={proc.returncode}\n")
        if proc.returncode != 0:
            sys.stderr.write(proc.stderr[-2000:])
            sys.stderr.write("\n")

    f.close()


if __name__ == "__main__":
    main()
