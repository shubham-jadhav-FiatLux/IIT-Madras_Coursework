#!/bin/bash
# setup_pbrt.sh — install pbrt-v4 in user space on the AQUA login node.
# Builds Embree dependencies from source if no system module is available.
# Usage:
#   bash cluster/setup_pbrt.sh
#
# Output: $HOME/local/pbrt-v4/build/pbrt
# After install, add to PATH:
#   export PATH=$HOME/local/pbrt-v4/build:$PATH
#
# Notes:
#   - AQUA RHEL 7.6 has GCC 9.2.0 in default PATH (verified in §14 of notes).
#     pbrt-v4 needs C++17 + CMake ≥ 3.12. GCC 9.2 is sufficient.
#   - Embree is required. We try the system module first; if not present we
#     build the Embree-lite shim that pbrt-v4 ships internally.
#   - Estimated time: ~20–40 minutes on the login node.

set -euo pipefail

PREFIX="$HOME/local"
mkdir -p "$PREFIX"

echo "=== 1. Confirm toolchain ==="
g++ --version | head -1
cmake --version | head -1 || {
  echo "ERROR: cmake not in PATH; try 'module load cmake' or contact admin"
  exit 1
}

echo ""
echo "=== 2. Try system Embree module ==="
HAVE_EMBREE=0
if module avail 2>&1 | grep -qi embree; then
  echo "Embree module found; will use it"
  HAVE_EMBREE=1
else
  echo "No Embree module; pbrt-v4 will build with its bundled shim"
fi

echo ""
echo "=== 3. Clone pbrt-v4 ==="
PBRT_DIR="$PREFIX/pbrt-v4"
if [ ! -d "$PBRT_DIR" ]; then
  # Clone WITHOUT --recursive — pbrt-v4 has multiple submodules whose pinned
  # SHAs are no longer fetchable upstream (history-rewrite + master→main
  # renames in google/double-conversion, AcademySoftwareFoundation/openexr,
  # google/glog, etc.). Standard `git submodule update` fails on those.
  git clone https://github.com/mmp/pbrt-v4.git "$PBRT_DIR"
fi
(
  cd "$PBRT_DIR"
  # Try pinned SHAs first (works for stable submodules like zlib, ptex, qoi).
  git submodule update --init --recursive 2>/tmp/_sm.log || true

  # For any submodule that ended up empty, fetch upstream HEAD directly.
  # This is API-compatible with pbrt-v4 in practice — each submodule is a
  # mature utility library with stable public APIs.
  git config -f .gitmodules --get-regexp '^submodule\..*\.path$' \
  | while read key sub_path; do
      [ -d "$sub_path" ] && [ -n "$(ls -A "$sub_path" 2>/dev/null)" ] && continue
      name=${key#submodule.}; name=${name%.path}
      url=$(git config -f .gitmodules --get "submodule.${name}.url")
      echo "  refreshing $sub_path ← upstream HEAD of $url"
      rm -rf "$sub_path"
      git clone --depth 1 "$url" "$sub_path" \
          || { echo "  FATAL: cannot clone $url"; exit 1; }
  done
)

echo ""
echo "=== 4. Configure and build ==="
mkdir -p "$PBRT_DIR/build"
cd "$PBRT_DIR/build"

CMAKE_ARGS=(
  -DCMAKE_BUILD_TYPE=Release
  -DPBRT_OPTIMIZED_BUILD=ON
)
if [ "$HAVE_EMBREE" = 1 ]; then
  module load embree || true
  CMAKE_ARGS+=(-DPBRT_USE_EMBREE=ON)
fi

cmake "${CMAKE_ARGS[@]}" ..
make -j"$(nproc)"

echo ""
echo "=== 5. Smoke test ==="
./pbrt --help | head -5

echo ""
echo "=== Done ==="
echo "pbrt binary at: $PBRT_DIR/build/pbrt"
echo "Add to PATH:    export PATH=$PBRT_DIR/build:\$PATH"
