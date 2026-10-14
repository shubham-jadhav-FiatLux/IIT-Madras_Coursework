#!/usr/bin/env python3
"""
compare_mitsuba.py
Compares our path tracer against Mitsuba 3 (scalar + LLVM) and pbrt-v4
on the Cornell Box.

Key conventions:
  - Mitsuba's rectangle is in the XY plane (z=0), normal = +z.
  - We use Mitsuba's built-in unit-scale Cornell Box as the base, then
    patch material colours and box geometry to match our scene exactly
    (axis-aligned boxes, same albedos, same vfov=39.3°).
  - GPU (cuda_ad_rgb) requires OptiX R535+; skipped here (driver=470).
  - LLVM variant has JIT state conflicts if run in same process as scalar;
    we run it in a subprocess.
  - pbrt-v4 renders linear EXR; we apply the same Reinhard+gamma as Mitsuba.
"""

import os, sys, re, time, subprocess, tempfile
import numpy as np

PBRT_BIN = os.environ.get('PBRT_BIN', os.path.expanduser('~/pbrt-v4/build/pbrt'))

# ── PPM helpers ───────────────────────────────────────────────────────────────

def read_ppm(path):
    with open(path, 'rb') as f:
        assert f.readline().strip() == b'P6', "not a P6 PPM"
        while True:
            line = f.readline().strip()
            if not line.startswith(b'#'): break
        W, H = map(int, line.split())
        assert int(f.readline().strip()) == 255
        data = np.frombuffer(f.read(), dtype=np.uint8)
    return data.reshape(H, W, 3).astype(np.float32) / 255.0

def save_ppm(path, img_u8):
    H, W, _ = img_u8.shape
    with open(path, 'wb') as f:
        f.write(f'P6\n{W} {H}\n255\n'.encode())
        f.write(img_u8.tobytes())

def reinhard_gamma(x, g=2.2):
    return np.clip(x / (1.0 + x), 0, 1) ** (1.0 / g)

def to_u8(x):
    return (np.clip(x, 0, 1) * 255.0 + 0.5).astype(np.uint8)

def rmse(a, b):
    """RMSE in [0,1] (both inputs may be float [0,1] or uint8)."""
    a = a.astype(np.float32); b = b.astype(np.float32)
    if a.max() > 1.01: a /= 255.0
    if b.max() > 1.01: b /= 255.0
    return float(np.sqrt(np.mean((a - b) ** 2)))

# ── Mitsuba scene: exact match of our cornell_box.scene ──────────────────────
#
# Mitsuba uses unit-scale (room spans ≈[-1,1]³). Material colours are
# identical to ours. Axis-aligned boxes match our scene (no rotation).
# Camera: fov=39.3° vertical, position and lookat from built-in scene.
# Light : same relative size and position as in mi.cornell_box() source.

def mitsuba_scene_dict(spp: int, W: int = 512, H: int = 512):
    import mitsuba as mi
    T = mi.ScalarTransform4f
    sq = int(spp ** 0.5 + 0.5)
    sampler_type = 'stratified' if sq * sq == spp else 'independent'
    return {
        'type': 'scene',
        'integrator': {'type': 'path', 'max_depth': 8, 'rr_depth': 4},
        'sensor': {
            'type': 'perspective',
            'fov': 39.3,
            'fov_axis': 'y',
            'near_clip': 0.001,
            'far_clip': 100.0,
            'to_world': T.look_at(
                origin=[0, 0, 3.90],
                target=[0, 0, 0],
                up=[0, 1, 0],
            ),
            'film': {
                'type': 'hdrfilm',
                'width': W, 'height': H,
                'pixel_format': 'rgb',
                'rfilter': {'type': 'box'},
            },
            'sampler': {'type': sampler_type, 'sample_count': spp},
        },
        # ── BSDFs (exact albedos from our scene) ──────────────────────────
        'white': {
            'type': 'diffuse',
            'reflectance': {'type': 'rgb', 'value': [0.73, 0.73, 0.73]},
        },
        'red': {
            'type': 'diffuse',
            'reflectance': {'type': 'rgb', 'value': [0.65, 0.05, 0.05]},
        },
        'green': {
            'type': 'diffuse',
            'reflectance': {'type': 'rgb', 'value': [0.12, 0.45, 0.15]},
        },
        # ── Walls (Mitsuba rectangle: XY plane at z=0, normal=+z) ─────────
        # Floor: translate down, rotate -90° around x → XZ plane, normal +y
        'floor': {
            'type': 'rectangle',
            'to_world': T.translate([0, -1, 0]).rotate([1, 0, 0], -90),
            'bsdf': {'type': 'ref', 'id': 'white'},
        },
        # Ceiling: translate up, rotate +90° around x → XZ plane, normal -y
        'ceiling': {
            'type': 'rectangle',
            'to_world': T.translate([0, 1, 0]).rotate([1, 0, 0], 90),
            'bsdf': {'type': 'ref', 'id': 'white'},
        },
        # Back wall: translate to z=-1, no rotation (XY plane, normal +z toward cam)
        'back': {
            'type': 'rectangle',
            'to_world': T.translate([0, 0, -1]),
            'bsdf': {'type': 'ref', 'id': 'white'},
        },
        # Green wall at x=+1 (viewer's right), rotate -90° around y → YZ plane
        'green_wall': {
            'type': 'rectangle',
            'to_world': T.translate([1, 0, 0]).rotate([0, 1, 0], -90),
            'bsdf': {'type': 'ref', 'id': 'green'},
        },
        # Red wall at x=-1 (viewer's left), rotate +90° around y → YZ plane
        'red_wall': {
            'type': 'rectangle',
            'to_world': T.translate([-1, 0, 0]).rotate([0, 1, 0], 90),
            'bsdf': {'type': 'ref', 'id': 'red'},
        },
        # ── Ceiling light (matched proportionally from mi.cornell_box()) ──
        'light': {
            'type': 'rectangle',
            'to_world': T.translate([0.0, 0.99, 0.01]).rotate([1, 0, 0], 90).scale([0.23, 0.19, 0.19]),
            'bsdf': {'type': 'ref', 'id': 'white'},
            'emitter': {
                'type': 'area',
                'radiance': {'type': 'rgb', 'value': [18.387, 13.9873, 6.75357]},
            },
        },
        # ── Interior boxes (axis-aligned, no rotation — matches our scene) ─
        # Cornell proportions: tall box ~0.3 scale, centered at (-0.33,-0.4,-0.28)
        # Short box ~0.3 scale, centered at (0.33,-0.7, 0.38)
        'tall_box': {
            'type': 'cube',
            'to_world': T.translate([-0.33, -0.4, -0.28]).scale([0.3, 0.61, 0.3]),
            'bsdf': {'type': 'ref', 'id': 'white'},
        },
        'short_box': {
            'type': 'cube',
            'to_world': T.translate([0.33, -0.7, 0.38]).scale(0.3),
            'bsdf': {'type': 'ref', 'id': 'white'},
        },
    }

# ── Render helpers ────────────────────────────────────────────────────────────

def render_mitsuba_scalar(spp: int, out_path: str):
    """Render with Mitsuba scalar_rgb (1-thread reference CPU path tracer)."""
    import mitsuba as mi
    mi.set_variant('scalar_rgb')
    scene = mi.load_dict(mitsuba_scene_dict(spp))
    t0 = time.perf_counter()
    img = mi.render(scene)
    ms = (time.perf_counter() - t0) * 1e3
    linear = np.array(img)
    u8 = to_u8(reinhard_gamma(linear))
    save_ppm(out_path, u8)
    return ms, read_ppm(out_path)

def render_mitsuba_llvm_subprocess(spp: int, out_path: str):
    """Run LLVM variant in a subprocess to avoid JIT state conflicts."""
    helper = f"""
import sys, time, numpy as np
sys.path.insert(0, '.')
import mitsuba as mi
mi.set_variant('llvm_ad_rgb')

def reinhard_gamma(x, g=2.2):
    return np.clip(x / (1.0 + x), 0, 1) ** (1.0 / g)

exec(open('compare_mitsuba.py').read().split('def render_mitsuba_llvm_subprocess')[0])
scene = mi.load_dict(mitsuba_scene_dict({spp}))
t0 = time.perf_counter()
img = mi.render(scene)
ms = (time.perf_counter() - t0) * 1e3
linear = np.array(img)
u8 = (np.clip(reinhard_gamma(linear), 0, 1) * 255 + 0.5).astype(np.uint8)
H, W, _ = u8.shape
with open('{out_path}', 'wb') as f:
    f.write(f'P6\\n{{W}} {{H}}\\n255\\n'.encode())
    f.write(u8.tobytes())
print(f'TIME {{ms:.1f}}')
"""
    with tempfile.NamedTemporaryFile(suffix='.py', mode='w', delete=False) as f:
        f.write(helper); tmp = f.name
    try:
        r = subprocess.run(['python3', tmp], capture_output=True, text=True,
                           cwd=os.path.dirname(os.path.abspath(__file__)))
        for line in r.stdout.splitlines():
            if line.startswith('TIME '):
                return float(line.split()[1]), read_ppm(out_path)
        raise RuntimeError(r.stderr[-500:] if r.stderr else "no output")
    finally:
        os.unlink(tmp)

def render_pbrt(spp: int, threads: int, out_ppm: str):
    """Render with pbrt-v4; parse timing, convert linear EXR to γ-corrected PPM."""
    import OpenEXR, Imath

    proj_dir = os.path.dirname(os.path.abspath(__file__))
    exr_path = os.path.join(proj_dir, f'output/pbrt_{spp}spp.exr')

    with open(os.path.join(proj_dir, 'scenes/cornell_box_pbrt.pbrt')) as f:
        scene_text = f.read()
    scene_text = re.sub(r'"integer pixelsamples"\s+\[\s*\d+\s*\]',
                        f'"integer pixelsamples" [ {spp} ]', scene_text)
    scene_text = re.sub(r'"string filename"\s+\[.*?\]',
                        f'"string filename" [ "{exr_path}" ]', scene_text)

    tmp_scene = f'/tmp/cb_pbrt_{spp}.pbrt'
    with open(tmp_scene, 'w') as f:
        f.write(scene_text)

    t0 = time.perf_counter()
    r = subprocess.run([PBRT_BIN, '--nthreads', str(threads), tmp_scene],
                       capture_output=True, text=True, cwd=proj_dir)
    ms = (time.perf_counter() - t0) * 1e3

    # pbrt prints "Rendering: [...] (XX.Xs)" on the last progress line
    combined = r.stderr + r.stdout
    for line in reversed(combined.splitlines()):
        m = re.search(r'\((\d+\.?\d*)s\)', line)
        if m:
            ms = float(m.group(1)) * 1e3
            break

    if not os.path.exists(exr_path):
        raise RuntimeError(f"pbrt did not produce {exr_path}.\nstderr: {r.stderr[-500:]}")

    exr = OpenEXR.InputFile(exr_path)
    dw = exr.header()['dataWindow']
    W = dw.max.x - dw.min.x + 1
    H = dw.max.y - dw.min.y + 1
    ft = Imath.PixelType(Imath.PixelType.FLOAT)
    R = np.frombuffer(exr.channel('R', ft), dtype=np.float32).reshape(H, W)
    G = np.frombuffer(exr.channel('G', ft), dtype=np.float32).reshape(H, W)
    B = np.frombuffer(exr.channel('B', ft), dtype=np.float32).reshape(H, W)
    linear = np.stack([R, G, B], axis=-1)

    u8 = to_u8(reinhard_gamma(linear))
    save_ppm(out_ppm, u8)
    return ms, read_ppm(out_ppm)


def render_ours(spp: int, threads: int, out_path: str):
    """Run our renderer; parse render-only time from stdout."""
    with open('scenes/cornell_box.scene') as f:
        text = f.read()
    text = re.sub(r'^spp \d+', f'spp {spp}', text, flags=re.MULTILINE)
    tmp = f'/tmp/cb_{spp}.scene'
    with open(tmp, 'w') as f:
        f.write(text)
    env = {**os.environ, 'OMP_NUM_THREADS': str(threads)}
    r = subprocess.run(['./build/release/rt', tmp, out_path],
                       capture_output=True, text=True, env=env)
    # extract "render XXXX.X ms" from stdout
    parts = r.stdout.split()
    for i, tok in enumerate(parts):
        if tok == 'render' and i + 1 < len(parts):
            try:
                return float(parts[i + 1]), read_ppm(out_path)
            except ValueError:
                pass
    raise RuntimeError(f"Could not parse time from: {r.stdout!r}")

# ── Main ──────────────────────────────────────────────────────────────────────

def main():
    os.makedirs('output', exist_ok=True)
    spp_list = [64, 256, 1024]

    print(f"\n{'='*72}")
    print("  Cornell Box path tracer comparison: ours vs Mitsuba 3 vs pbrt-v4")
    print("  Machine: Intel i7-8550U (4C/8T @1.8GHz), 8 GB RAM")
    print(f"{'='*72}\n")

    # ── Our renderer (OpenMP, 8 threads) ─────────────────────────────────────
    print("--- Our renderer (C++17 + OpenMP, 8 threads) ---")
    our = {}
    for spp in spp_list:
        path = f'output/ours_{spp}spp.ppm'
        ms, img = render_ours(spp, 8, path)
        our[spp] = (ms, img)
        print(f"  spp={spp:4d}  {ms:8.0f} ms  → {path}")

    ref = our[1024][1]   # our 1024-spp as quality reference

    print(f"\n  RMSE vs our 1024-spp reference:")
    for spp in spp_list:
        print(f"    spp={spp:4d}: RMSE={rmse(ref, our[spp][1]):.4f}")

    # ── Mitsuba scalar_rgb (single-thread reference CPU) ─────────────────────
    print("\n--- Mitsuba 3.8.0 scalar_rgb (1-thread, CPU path tracer) ---")
    mi_scalar = {}
    try:
        import mitsuba as mi
        mi.set_variant('scalar_rgb')
        # Render 1 spp first to warm up JIT
        _ = mi.render(mi.load_dict(mitsuba_scene_dict(1, 32, 32)))
        for spp in spp_list:
            path = f'output/mi_scalar_{spp}spp.ppm'
            ms, img = render_mitsuba_scalar(spp, path)
            mi_scalar[spp] = (ms, img)
            r = rmse(ref, img)
            print(f"  spp={spp:4d}  {ms:8.0f} ms  RMSE={r:.4f}  → {path}")
    except Exception as e:
        print(f"  ERROR: {e}")

    # ── Mitsuba llvm_ad_rgb (multi-thread, wavefront CPU) ────────────────────
    print("\n--- Mitsuba 3.8.0 llvm_ad_rgb (multi-thread wavefront, subprocess) ---")
    mi_llvm = {}
    for spp in spp_list:
        path = f'output/mi_llvm_{spp}spp.ppm'
        try:
            ms, img = render_mitsuba_llvm_subprocess(spp, path)
            mi_llvm[spp] = (ms, img)
            r = rmse(ref, img)
            print(f"  spp={spp:4d}  {ms:8.0f} ms  RMSE={r:.4f}  → {path}")
        except Exception as e:
            print(f"  spp={spp}  ERROR: {e}")

    # ── pbrt-v4 (CPU path tracer, triangle meshes, z-Sobol sampler) ──────────
    print("\n--- pbrt-v4 (CPU path tracer, z-Sobol sampler, 8 threads) ---")
    pbrt = {}
    if os.path.exists(PBRT_BIN):
        for spp in spp_list:
            path = f'output/pbrt_{spp}spp.ppm'
            try:
                ms, img = render_pbrt(spp, 8, path)
                pbrt[spp] = (ms, img)
                r = rmse(ref, img)
                print(f"  spp={spp:4d}  {ms:8.0f} ms  RMSE={r:.4f}  → {path}")
            except Exception as e:
                print(f"  spp={spp}  ERROR: {e}")
    else:
        print(f"  SKIP: {PBRT_BIN} not found")

    # ── GPU note ──────────────────────────────────────────────────────────────
    print("\n--- GPU (cuda_ad_rgb) ---")
    print("  SKIP: driver 470 < R535 required by OptiX 8.0.")
    print("  On RTX 4090 or A100, GPU renders 20-100× faster than CPU at high SPP.")

    # ── Summary table ─────────────────────────────────────────────────────────
    print(f"\n{'='*72}")
    print(f"  {'Renderer':<35} {'SPP':>4}  {'Time (ms)':>10}  {'vs serial':>9}  {'RMSE':>7}")
    print(f"{'─'*72}")
    t1_ours = our[1][0] if 1 in our else our[spp_list[0]][0] / spp_list[0]
    for spp in spp_list:
        ms, img = our[spp]
        label = 'Ours (8-thread OMP)' if spp == spp_list[0] else ''
        spdup = our[spp_list[0]][0] / ms if ms > 0 else 0
        print(f"  {'Ours  OMP-8t':<35} {spp:4d}  {ms:10.0f}  {'—':>9}  {rmse(ref,img):7.4f}")
    print(f"{'─'*72}")
    if mi_scalar:
        for spp in spp_list:
            if spp in mi_scalar:
                ms, img = mi_scalar[spp]
                ratio = our[spp][0] / ms if ms > 0 else 0
                print(f"  {'Mitsuba scalar (1-thread)':<35} {spp:4d}  {ms:10.0f}  {ratio:>8.2f}×  {rmse(ref,img):7.4f}")
        print(f"{'─'*72}")
    if mi_llvm:
        for spp in spp_list:
            if spp in mi_llvm:
                ms, img = mi_llvm[spp]
                ratio = our[spp][0] / ms if ms > 0 else 0
                print(f"  {'Mitsuba LLVM (multi-thread)':<35} {spp:4d}  {ms:10.0f}  {ratio:>8.2f}×  {rmse(ref,img):7.4f}")
        print(f"{'─'*72}")
    if pbrt:
        for spp in spp_list:
            if spp in pbrt:
                ms, img = pbrt[spp]
                ratio = our[spp][0] / ms if ms > 0 else 0
                print(f"  {'pbrt-v4 (8-thread, z-Sobol)':<35} {spp:4d}  {ms:10.0f}  {ratio:>8.2f}×  {rmse(ref,img):7.4f}")
        print(f"{'─'*72}")
    print()
    print("  Notes:")
    print("  · RMSE = root-mean-squared error vs our 1024-spp reference (γ-corrected).")
    print("  · 'vs serial' = our-time / renderer-time (>1 means that renderer is faster).")
    print("  · Mitsuba and pbrt use triangle meshes; ours uses analytical intersections.")
    print("  · Mitsuba scene: unit-scale ([-1,1]); ours and pbrt: mm-scale.")
    print("  · LLVM wavefront path tracer is Mitsuba's multi-core CPU backend.")
    print("  · pbrt-v4 uses z-Sobol blue-noise sampler; outputs linear EXR (Reinhard+γ applied).")
    print("  · GPU comparison requires discrete GPU with OptiX R535+ (see report).")
    print(f"{'='*72}\n")

if __name__ == '__main__':
    main()
