#!/bin/bash
# aqua_env.sh — single source of truth for the AQUA cluster environment.
# Source this from your shell profile, your interactive sessions, and from
# the PBS job script. It is idempotent and defensive: re-sourcing is safe,
# and it works whether or not the module system is initialised.
#
# Resolves:
#   gcc 9.2.0     — /lfs/sware/gcc920    (system /usr/bin/gcc is 4.8.5, too old)
#   OpenMPI 4.1.5 — /lfs/sware/openmpi415
#   Anaconda 3    — /lfs/sware/anaconda3_2023.09 (python385 lacks SSL)
#   Mitsuba 3     — pip-installed in $HOME/local/mitsuba-venv
#   Dr.Jit LLVM   — /opt/intel/.../libLLVM-11.so for llvm_ad_rgb variant
#
# Defensive layout (matters in PBS jobs which don't inherit the login shell):
#   1. Try to bootstrap the `module` function if it isn't already defined
#   2. Try `module load` for each dependency (silently OK if it fails)
#   3. Explicitly prepend each component's bin/ to PATH using absolute paths
#      — this is the layer that actually guarantees PATH is correct
#   4. Activate the Mitsuba venv (its python3 is a symlink we need on PATH)
#
# Module-name decisions (empirically validated 2026-05-06):
#   * anaconda3_2023.09 (not python385) — python385 lacks SSL → no pip
#   * gcc920 (not gcc13.3.0) — system tooling expects 9.x ABI
#   * openmpi415 (not Intel MPI) — wraps gcc 9.2.0 cleanly

# 1. Try to source the module bootstrap if `module` isn't defined.
if ! command -v module >/dev/null 2>&1; then
    for f in /etc/profile.d/modules.sh /usr/share/Modules/init/bash; do
        if [ -f "$f" ]; then
            # shellcheck disable=SC1090
            source "$f"
            break
        fi
    done
fi

# 2. Module loads — best-effort; absolute-path PATH below is the real fix.
if command -v module >/dev/null 2>&1; then
    # Don't `module purge`: in some PBS configurations it removes the
    # default site bootstrap modules and leaves the shell broken.
    module load anaconda3_2023.09 2>/dev/null || true
    module load gcc920            2>/dev/null || true
    module load openmpi415        2>/dev/null || true
fi

# 3. Belt-and-suspenders: prepend each component's bin/ explicitly. This
#    makes the env work even when the module system silently no-ops, which
#    is what bit us at TIER 1.1 of the first job submission.
__rt_prepend() { [ -d "$1" ] && export PATH="$1:$PATH"; }

# Real on-disk names on AQUA: gcc9.2.0 (with dots), anaconda3_2023 (no .09 suffix).
# The earlier hard-coded names (gcc920, anaconda3_2023.09) were module names,
# not directory names — they no-op'd silently for non-interactive shells.
__rt_prepend /lfs/sware/anaconda3_2023/bin
__rt_prepend /lfs/sware/gcc9.2.0/bin
__rt_prepend /lfs/sware/openmpi415/bin
__rt_prepend "$HOME/local/bin"
__rt_prepend "$HOME/local/mitsuba-venv/bin"
__rt_prepend "$HOME/local/pbrt-v4/build"

# Same idea for LD_LIBRARY_PATH (gcc 9.2.0 libstdc++ + libgomp must come first
# so the binary picks them up at runtime, not the system 4.8.5 versions).
__rt_prepend_lib() { [ -d "$1" ] && export LD_LIBRARY_PATH="$1:${LD_LIBRARY_PATH:-}"; }
__rt_prepend_lib /lfs/sware/gcc9.2.0/lib64
__rt_prepend_lib /lfs/sware/openmpi415/lib

unset -f __rt_prepend __rt_prepend_lib

# 4. Activate the Mitsuba venv (sets VIRTUAL_ENV, prepends bin to PATH).
if [ -f "$HOME/local/mitsuba-venv/bin/activate" ]; then
    # shellcheck disable=SC1091
    source "$HOME/local/mitsuba-venv/bin/activate"
fi

# Dr.Jit needs an external libLLVM ≥ 11. The /opt/intel path exists ONLY on
# the login node (local disk); compute nodes (cn*) don't mount it, which
# silently breaks Mitsuba LLVM during PBS jobs (jitc_llvm_init fails).
# /lfs/sware/ is the global filesystem visible from every node.
for __rt_llvm in \
    /lfs/sware/anaconda3_2023/lib/libLLVM-11.so \
    /opt/intel/oneapi/intelpython/python3.9/lib/libLLVM-11.so; do
    if [ -f "$__rt_llvm" ]; then
        export DRJIT_LIBLLVM_PATH="$__rt_llvm"
        break
    fi
done
unset __rt_llvm

# Diagnostic helper — call this in the PBS job to abort early if PATH wrong.
rt_env_check() {
    local fail=0
    for cmd in g++ mpicxx python3; do
        if ! command -v "$cmd" >/dev/null 2>&1; then
            echo "rt_env_check: missing $cmd on PATH" >&2
            fail=1
        fi
    done
    # The system /usr/bin/g++ is 4.8.5, which can't compile -std=c++17.
    # If our gcc 9.2.0 isn't winning the PATH race, fail fast.
    if command -v g++ >/dev/null 2>&1; then
        local gxx_ver
        gxx_ver=$(g++ -dumpversion 2>/dev/null | cut -d. -f1)
        if [ -z "$gxx_ver" ] || [ "$gxx_ver" -lt 9 ]; then
            echo "rt_env_check: g++ at $(command -v g++) reports version $(g++ -dumpversion 2>/dev/null) — need >= 9.x" >&2
            fail=1
        fi
    fi
    if [ -n "${DRJIT_LIBLLVM_PATH:-}" ] && [ ! -f "$DRJIT_LIBLLVM_PATH" ]; then
        echo "rt_env_check: DRJIT_LIBLLVM_PATH=$DRJIT_LIBLLVM_PATH not visible on $(hostname)" >&2
        fail=1
    fi
    if [ "$fail" = 1 ]; then
        echo "rt_env_check: PATH=$PATH" >&2
        echo "rt_env_check: failing — fix aqua_env.sh paths or re-run setup" >&2
        return 1
    fi
    return 0
}
