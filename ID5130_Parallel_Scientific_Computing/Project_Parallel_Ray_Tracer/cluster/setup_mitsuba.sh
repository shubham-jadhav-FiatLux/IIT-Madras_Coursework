#!/bin/bash
# setup_mitsuba.sh — install Mitsuba 3 in user-space on the AQUA login node.
#
# Empirically validated on AQUA (May 2026):
#   * AQUA's `python385` module lacks SSL → cannot reach pypi. Use the
#     `anaconda3_2023.09` module instead (loaded by cluster/aqua_env.sh).
#   * Numpy 2.x requires GCC ≥ 9.3 to build from source; AQUA has 9.2.
#     We pin to numpy 1.26.x with `--only-binary=:all:` to force wheels.
#   * Dr.Jit's `llvm_ad_rgb` variant needs an external libLLVM ≥ 11. The
#     Intel oneAPI install ships one at /opt/intel/.../libLLVM-11.so;
#     `cluster/aqua_env.sh` sets DRJIT_LIBLLVM_PATH for it.
#
# Usage:
#   source cluster/aqua_env.sh         # gets the right Anaconda + libLLVM
#   bash   cluster/setup_mitsuba.sh

set -euo pipefail

PREFIX="$HOME/local"
VENV="$PREFIX/mitsuba-venv"
mkdir -p "$PREFIX" "$PREFIX/bin"

echo "=== 1. Confirm Python (≥ 3.8 with SSL) ==="
python3 --version || { echo "ERROR: python3 missing — source cluster/aqua_env.sh first"; exit 1; }
python3 -c "import ssl; print('SSL', ssl.OPENSSL_VERSION)" || {
  echo "ERROR: Python lacks SSL — wrong module loaded; aqua_env.sh expects anaconda3"
  exit 1
}

echo ""
echo "=== 2. Create virtual environment ==="
if [ ! -d "$VENV" ]; then
  python3 -m venv "$VENV"
fi
# shellcheck disable=SC1090
source "$VENV/bin/activate"
pip install --upgrade pip

echo ""
echo "=== 3. Install Mitsuba 3 + analysis deps (binary wheels only) ==="
# --only-binary forces pip to refuse source builds; numpy<2 has wheels for
# all supported pythons and doesn't need GCC ≥ 9.3.
pip install --only-binary=:all: \
    "numpy==1.26.4" "scipy<1.13" mitsuba imageio "scikit-image<0.23"

echo ""
echo "=== 4. Smoke test ==="
python3 -c "
import mitsuba as mi
mi.set_variant('scalar_rgb')
print('Mitsuba', mi.__version__, 'scalar_rgb OK')
print('Available variants:', mi.variants())
"

# Wrapper for non-venv invocation (e.g., via job script that hasn't
# sourced aqua_env.sh yet).
WRAPPER="$PREFIX/bin/mitsuba"
cat > "$WRAPPER" << EOF
#!/bin/bash
source "$VENV/bin/activate"
# Dr.Jit's LLVM backend; harmless if already set.
: "\${DRJIT_LIBLLVM_PATH:=/opt/intel/oneapi/intelpython/python3.9/lib/libLLVM-11.so}"
export DRJIT_LIBLLVM_PATH
exec python3 -m mitsuba "\$@"
EOF
chmod +x "$WRAPPER"

echo ""
echo "=== Done ==="
echo "Activate manually:   source $VENV/bin/activate"
echo "Wrapper installed:   $WRAPPER (already on PATH via aqua_env.sh)"
echo ""
echo "Test the LLVM variant:"
echo "  python3 -c \"import mitsuba as mi; mi.set_variant('llvm_ad_rgb'); print('LLVM OK')\""
