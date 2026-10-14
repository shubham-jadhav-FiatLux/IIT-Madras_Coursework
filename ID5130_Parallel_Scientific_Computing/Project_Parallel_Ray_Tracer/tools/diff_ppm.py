#!/usr/bin/env python3
"""
diff_ppm.py — quality oracle for renders.

Compares two images (PPM/PNG/EXR) and reports:
  RMSE        — root-mean-squared error per channel, normalized to [0,1]
  bias        — signed mean(diff) per channel
  max_abs     — max absolute pixel difference
  SSIM        — structural similarity (skimage; optional)
  FLIP        — perceptual metric (NVIDIA flip CLI; optional)

Usage:
  diff_ppm.py <render.ppm> <reference.{ppm,exr}>
  diff_ppm.py --json <a> <b>          # machine-readable
  diff_ppm.py --threshold 0.01 <a> <b>  # exit 1 if RMSE exceeds threshold

Designed to slot into a Makefile target (`make verify`) and into the
benchmark harness CSV pipeline.
"""
from __future__ import annotations

import argparse
import json
import os
import struct
import subprocess
import sys
from pathlib import Path

import numpy as np


def load_ppm(path: Path) -> np.ndarray:
    """Load P6 binary PPM. Returns float array shape (H, W, 3) in [0, 1]."""
    with open(path, "rb") as f:
        magic = f.readline().strip()
        if magic != b"P6":
            raise ValueError(f"{path}: not a P6 PPM (got {magic!r})")
        # Skip comments
        while True:
            line = f.readline()
            if not line.startswith(b"#"):
                break
        w, h = map(int, line.split())
        maxval = int(f.readline().strip())
        bytes_per_chan = 1 if maxval < 256 else 2
        data = f.read(w * h * 3 * bytes_per_chan)
        if bytes_per_chan == 1:
            arr = np.frombuffer(data, dtype=np.uint8).astype(np.float32) / maxval
        else:
            arr = np.frombuffer(data, dtype=">u2").astype(np.float32) / maxval
        return arr.reshape(h, w, 3)


def load_exr(path: Path) -> np.ndarray:
    """Load EXR via OpenEXR or imageio. Returns float array (H, W, 3)."""
    try:
        import imageio.v3 as iio
        img = iio.imread(path)
        if img.ndim == 2:
            img = np.stack([img] * 3, axis=-1)
        if img.shape[-1] == 4:
            img = img[..., :3]
        return img.astype(np.float32)
    except ImportError:
        raise RuntimeError("imageio not available; pip install imageio[freeimage]")


def load_png(path: Path) -> np.ndarray:
    try:
        import imageio.v3 as iio
        img = iio.imread(path).astype(np.float32) / 255.0
        if img.ndim == 2:
            img = np.stack([img] * 3, axis=-1)
        if img.shape[-1] == 4:
            img = img[..., :3]
        return img
    except ImportError:
        raise RuntimeError("imageio not available; pip install imageio")


def load_image(path: Path) -> np.ndarray:
    s = str(path).lower()
    if s.endswith(".ppm"):
        return load_ppm(path)
    if s.endswith(".exr"):
        return load_exr(path)
    if s.endswith(".png"):
        return load_png(path)
    raise ValueError(f"unsupported extension: {path}")


def rmse(a: np.ndarray, b: np.ndarray) -> float:
    return float(np.sqrt(np.mean((a - b) ** 2)))


def bias(a: np.ndarray, b: np.ndarray) -> np.ndarray:
    return np.mean(a - b, axis=(0, 1))


def max_abs(a: np.ndarray, b: np.ndarray) -> float:
    return float(np.max(np.abs(a - b)))


def try_ssim(a: np.ndarray, b: np.ndarray) -> float | None:
    try:
        from skimage.metrics import structural_similarity
        return float(structural_similarity(a, b, channel_axis=-1, data_range=1.0))
    except ImportError:
        return None


def try_flip(a_path: Path, b_path: Path) -> float | None:
    """Run NVIDIA flip CLI if available. Returns mean FLIP score."""
    if not any(Path(p).name == "flip" for p in os.environ.get("PATH", "").split(":")):
        return None
    try:
        out = subprocess.run(
            ["flip", "-r", str(b_path), "-t", str(a_path), "--no-error-map"],
            capture_output=True, text=True, timeout=60,
        )
        if out.returncode != 0:
            return None
        # flip prints "Mean: 0.0123" somewhere
        for line in out.stdout.splitlines():
            if "Mean" in line:
                return float(line.split(":")[-1].strip())
    except (subprocess.TimeoutExpired, FileNotFoundError, ValueError):
        pass
    return None


def compare(render_path: Path, ref_path: Path) -> dict:
    a = load_image(render_path)
    b = load_image(ref_path)
    if a.shape != b.shape:
        raise ValueError(f"shape mismatch: {a.shape} vs {b.shape}")
    result = {
        "render": str(render_path),
        "reference": str(ref_path),
        "shape": list(a.shape),
        "rmse": rmse(a, b),
        "bias_r": float(bias(a, b)[0]),
        "bias_g": float(bias(a, b)[1]),
        "bias_b": float(bias(a, b)[2]),
        "max_abs": max_abs(a, b),
        "ssim": try_ssim(a, b),
        "flip": try_flip(render_path, ref_path),
    }
    return result


def main():
    ap = argparse.ArgumentParser(description="Compare rendered image to reference")
    ap.add_argument("render", type=Path)
    ap.add_argument("reference", type=Path)
    ap.add_argument("--json", action="store_true", help="emit JSON")
    ap.add_argument("--threshold", type=float, default=None,
                    help="exit 1 if RMSE exceeds this value")
    args = ap.parse_args()

    res = compare(args.render, args.reference)

    if args.json:
        print(json.dumps(res, indent=2))
    else:
        print(f"render:    {res['render']}")
        print(f"reference: {res['reference']}")
        print(f"shape:     {res['shape']}")
        print(f"RMSE:      {res['rmse']:.6f}")
        print(f"bias:      R={res['bias_r']:+.6f}  G={res['bias_g']:+.6f}  B={res['bias_b']:+.6f}")
        print(f"max abs:   {res['max_abs']:.6f}")
        if res["ssim"] is not None:
            print(f"SSIM:      {res['ssim']:.6f}")
        if res["flip"] is not None:
            print(f"FLIP:      {res['flip']:.6f}")

    if args.threshold is not None and res["rmse"] > args.threshold:
        sys.stderr.write(
            f"\nFAIL: RMSE {res['rmse']:.6f} > threshold {args.threshold}\n"
        )
        sys.exit(1)


if __name__ == "__main__":
    main()
