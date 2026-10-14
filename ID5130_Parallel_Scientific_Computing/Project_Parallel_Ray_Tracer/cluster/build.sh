#!/bin/bash
# Run once on AQUA login node to build both binaries.
# Usage: bash cluster/build.sh
#
# Uses default environment (GCC 9.2.0 + Intel MPI 2019) which is already in PATH.
# Optionally load gcc13.3.0 for better AVX-512 codegen on Cascade Lake.

set -e
cd "$(dirname "$0")/.."   # project root

# Optional: uncomment for GCC 13 (better AVX-512 auto-vectorisation on Gold 6248)
# module load gcc13.3.0

echo "Compiler: $(g++ --version | head -1)"
echo "MPI:      $(mpicxx --version | head -1)"
echo "OpenMP:   $(g++ -fopenmp -dM -E - < /dev/null 2>/dev/null | grep OMP_VERSION | head -1)"

# Build OpenMP binary (rt)
make MODE=release

# Build MPI binary (rt_mpi)
make mpi MODE=release

echo ""
echo "Built:"
ls -lh build/release/rt build/release/rt_mpi
