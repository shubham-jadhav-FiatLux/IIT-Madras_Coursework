#!/bin/bash
# Run this on the AQUA cluster to check all requirements for the ray tracer project.
# Usage: bash check_aqua.sh

echo "=== 1. OS and kernel ==="
uname -srm
cat /etc/os-release 2>/dev/null | grep -E "^NAME=|^VERSION=" | head -2

echo ""
echo "=== 2. Job scheduler ==="
which sbatch 2>/dev/null && sbatch --version 2>/dev/null || echo "SLURM: not found"
which qsub   2>/dev/null && echo "PBS/Torque found" || echo "PBS: not found"

echo ""
echo "=== 3. Available modules (GCC, MPI) ==="
module avail 2>&1 | grep -iE "gcc|gnu|mpi|openmpi|mpich|intelmpi" | head -30

echo ""
echo "=== 4. Default compiler ==="
which g++ 2>/dev/null && g++ --version | head -1 || echo "g++ not in PATH"
which mpicxx 2>/dev/null && mpicxx --version | head -1 || echo "mpicxx not in PATH"
which mpirun 2>/dev/null && mpirun --version | head -2 || echo "mpirun not in PATH"

echo ""
echo "=== 5. C++17 + OpenMP compile test ==="
cat > /tmp/_rt_check.cpp << 'EOF'
#include <omp.h>
#include <cstdio>
#include <optional>
int main(){
    int n=0;
    #pragma omp parallel reduction(+:n)
    { n += omp_get_thread_num(); }
    printf("C++17 OK  OMP_threads=%d\n", omp_get_max_threads());
    return 0;
}
EOF
g++ -std=c++17 -fopenmp /tmp/_rt_check.cpp -o /tmp/_rt_check 2>&1
if [ $? -eq 0 ]; then
    /tmp/_rt_check
else
    echo "FAIL: check module load for gcc"
fi

echo ""
echo "=== 6. MPI + OpenMP compile test ==="
cat > /tmp/_mpi_check.cpp << 'EOF'
#include <mpi.h>
#include <omp.h>
#include <cstdio>
int main(int argc, char** argv){
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if(rank==0)
        printf("MPI OK  ranks=%d  OMP_threads=%d\n", size, omp_get_max_threads());
    MPI_Finalize();
    return 0;
}
EOF
mpicxx -std=c++17 -fopenmp /tmp/_mpi_check.cpp -o /tmp/_mpi_check 2>&1
if [ $? -eq 0 ]; then
    mpirun -n 2 /tmp/_mpi_check
else
    echo "FAIL: check module load for mpi"
fi

echo ""
echo "=== 7. CPU topology ==="
lscpu | grep -E "Model name|Socket|Core\(s\)|Thread|NUMA|^CPU\(s\):|L3"

echo ""
echo "=== 8. Memory ==="
free -h | head -2

echo ""
echo "=== 9. Disk / scratch ==="
df -h "$HOME" | tail -1
df -h /scratch 2>/dev/null | tail -1 || echo "/scratch not found"

echo ""
echo "=== 10. InfiniBand ==="
ls /sys/class/infiniband/ 2>/dev/null && echo "InfiniBand present" || echo "No InfiniBand sysfs"
ibstat 2>/dev/null | grep -E "Port State|Rate" | head -4 || true

echo ""
echo "=== 11. Queue / partition info ==="
sinfo 2>/dev/null | head -20 || qstat -q 2>/dev/null | head -20 || echo "No scheduler info"

echo ""
echo "=== DONE ==="
