#!/bin/bash
# ID5130 Assignment 1 - Full Test Suite
# Usage: bash run_all.sh
# Compiles all programs and runs every test case required by the assignment.

echo "=============================================="
echo "  ID5130 Assignment 1 - Full Test Run"
echo "=============================================="
echo ""

# Compile
echo "Compiling..."
make clean && make all
if [ $? -ne 0 ]; then echo "Compilation failed!"; exit 1; fi
echo ""

# ---- QUESTION 1(a): Riemann Integration ----
echo "======================================================"
echo "  Q1(a): Riemann Integration (Nx=48, Ny=36, Nz=24)"
echo "======================================================"
for p in 2 4 8; do
    echo "--- p = $p ---"
    ./q1a_riemann $p
done

# ---- QUESTION 1(b): Monte Carlo Integration ----
echo "======================================================"
echo "  Q1(b): Monte Carlo Integration"
echo "======================================================"
for p in 2 4 8; do
    echo "--- p = $p ---"
    ./q1b_montecarlo $p
done

# ---- QUESTION 2(a)(b)(c): Jacobi - delta=0.1 ----
echo "======================================================"
echo "  Q2(a)(b)(c): Jacobi, delta=0.1, verification"
echo "======================================================"
./q2_jacobi 8 0.1

# ---- QUESTION 2(c): Time vs Grid Size ----
echo "======================================================"
echo "  Q2(c): Jacobi, delta=0.01, 8 threads"
echo "======================================================"
./q2_jacobi 8 0.01

echo "======================================================"
echo "  Q2(c): Jacobi, delta=0.005, 8 threads"
echo "======================================================"
./q2_jacobi 8 0.005

# ---- QUESTION 2(d): Time vs Threads (delta=0.005) ----
echo "======================================================"
echo "  Q2(d): Jacobi, delta=0.005, varying threads"
echo "======================================================"
for p in 2 4 8 16; do
    echo "--- p = $p ---"
    ./q2_jacobi $p 0.005
done

echo ""
echo "======================================================"
echo "  All runs complete!"
echo "  CSV files generated for plotting."
echo "  Run: gnuplot plot_all.gp to generate plots."
echo "======================================================"
