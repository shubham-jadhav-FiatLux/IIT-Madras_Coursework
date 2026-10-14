#!/bin/bash
# ID5130 Assignment 2 - Full Run Script
# Open MPI refuses more ranks than cores unless told otherwise
MPIRUN="mpirun"
mpirun --version 2>&1 | grep -qi "open mpi" && MPIRUN="mpirun --oversubscribe"

echo "Compiling..."
make clean && make all || exit 1

echo ""
echo "======== Q1: Wave Equation ========"
echo "=== Q1(a): Serial ==="
./q1_serial

echo "=== Q1(b): MPI p=2 ==="
$MPIRUN -np 2 ./q1_mpi

echo "=== Q1(b): MPI p=4 ==="
$MPIRUN -np 4 ./q1_mpi

echo ""
echo "======== Q2: Poisson Equation ========"
echo "=== Q2(a): Serial delta=0.1 ==="
./q2_serial 0.1

echo "=== Q2(a/b): Serial delta=0.01 ==="
./q2_serial 0.01

echo "=== Q2(b): MPI p=2, delta=0.01 ==="
$MPIRUN -np 2 ./q2_mpi 0.01

echo "=== Q2(b): MPI p=4, delta=0.01 ==="
$MPIRUN -np 4 ./q2_mpi 0.01

echo "=== Q2(b): MPI p=8, delta=0.01 ==="
$MPIRUN -np 8 ./q2_mpi 0.01

echo ""
echo "======== Generating plots ========"
gnuplot plot_all.gp
gnuplot plot_q2.gp

echo ""
echo "Done! Check plot*.png files."
