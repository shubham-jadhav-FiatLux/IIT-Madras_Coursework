#!/usr/bin/gnuplot
# ID5130 Assignment 2 — All Plots

set terminal pngcairo enhanced size 1100,750 font "Helvetica,14" lw 1.5
set datafile separator ","

C1 = "#2166AC"
C2 = "#D6604D"
C3 = "#2CA02C"
C4 = "#984EA3"
CG = "#AAAAAA"
C5 = "#E69F00"

set style line 100 lt 1 lc rgb "#E8E8E8" lw 0.4
set grid back ls 100
set border 3 lw 1.2
set tics nomirror
set xrange [0:2]
set yrange [-0.3:1.15]
set xlabel "x"
set ylabel "u(x, t)"

# --- PLOT 1: Serial Upwind ---
set output "plot1_serial_upwind.png"
set title "{/=16 Q1(a) Serial: Upwind — t = 0, 0.5, 1.0}"
set key top right spacing 1.2
plot \
  "serial_upwind_t0.0.csv" u 1:3 skip 1 w l lw 2 lc rgb C1 dt 2 t "Exact t=0", \
  "serial_upwind_t0.0.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C2 t "Upwind t=0", \
  "serial_upwind_t0.5.csv" u 1:3 skip 1 w l lw 2 lc rgb C3 dt 2 t "Exact t=0.5", \
  "serial_upwind_t0.5.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C4 t "Upwind t=0.5", \
  "serial_upwind_t1.0.csv" u 1:3 skip 1 w l lw 2 lc rgb CG dt 2 t "Exact t=1.0", \
  "serial_upwind_t1.0.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C5 t "Upwind t=1.0"
print "  [1/7] plot1_serial_upwind.png"

# --- PLOT 2: Serial QUICK ---
set output "plot2_serial_quick.png"
set title "{/=16 Q1(a) Serial: QUICK — t = 0, 0.5, 1.0}"
plot \
  "serial_quick_t0.0.csv" u 1:3 skip 1 w l lw 2 lc rgb C1 dt 2 t "Exact t=0", \
  "serial_quick_t0.0.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C2 t "QUICK t=0", \
  "serial_quick_t0.5.csv" u 1:3 skip 1 w l lw 2 lc rgb C3 dt 2 t "Exact t=0.5", \
  "serial_quick_t0.5.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C4 t "QUICK t=0.5", \
  "serial_quick_t1.0.csv" u 1:3 skip 1 w l lw 2 lc rgb CG dt 2 t "Exact t=1.0", \
  "serial_quick_t1.0.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C5 t "QUICK t=1.0"
print "  [2/7] plot2_serial_quick.png"

# --- PLOT 3: Upwind vs QUICK comparison at t=1.0 (zoomed) ---
set output "plot3_upwind_vs_quick_t1.png"
set title "{/=16 Q1(c): Upwind vs QUICK at t = 1.0}"
set xrange [0.5:1.6]
plot \
  "serial_upwind_t1.0.csv" u 1:3 skip 1 w l lw 2.5 lc rgb C1 t "Exact", \
  "serial_upwind_t1.0.csv" u 1:2 skip 1 w l lw 1.8 lc rgb C2 t "Upwind", \
  "serial_quick_t1.0.csv"  u 1:2 skip 1 w l lw 1.8 lc rgb C3 t "QUICK"
print "  [3/7] plot3_upwind_vs_quick_t1.png"

# --- PLOT 4: MPI p=2 Upwind ---
set output "plot4_mpi_p2_upwind.png"
set xrange [0:2]
set title "{/=16 Q1(b) MPI p=2: Upwind — t = 0, 0.5, 1.0}"
plot \
  "mpi_p2_upwind_t0.0.csv" u 1:3 skip 1 w l lw 2 lc rgb C1 dt 2 t "Exact t=0", \
  "mpi_p2_upwind_t0.0.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C2 t "Upwind t=0", \
  "mpi_p2_upwind_t0.5.csv" u 1:3 skip 1 w l lw 2 lc rgb C3 dt 2 t "Exact t=0.5", \
  "mpi_p2_upwind_t0.5.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C4 t "Upwind t=0.5", \
  "mpi_p2_upwind_t1.0.csv" u 1:3 skip 1 w l lw 2 lc rgb CG dt 2 t "Exact t=1.0", \
  "mpi_p2_upwind_t1.0.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C5 t "Upwind t=1.0"
print "  [4/7] plot4_mpi_p2_upwind.png"

# --- PLOT 5: MPI p=2 QUICK ---
set output "plot5_mpi_p2_quick.png"
set title "{/=16 Q1(b) MPI p=2: QUICK — t = 0, 0.5, 1.0}"
plot \
  "mpi_p2_quick_t0.0.csv" u 1:3 skip 1 w l lw 2 lc rgb C1 dt 2 t "Exact t=0", \
  "mpi_p2_quick_t0.0.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C2 t "QUICK t=0", \
  "mpi_p2_quick_t0.5.csv" u 1:3 skip 1 w l lw 2 lc rgb C3 dt 2 t "Exact t=0.5", \
  "mpi_p2_quick_t0.5.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C4 t "QUICK t=0.5", \
  "mpi_p2_quick_t1.0.csv" u 1:3 skip 1 w l lw 2 lc rgb CG dt 2 t "Exact t=1.0", \
  "mpi_p2_quick_t1.0.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C5 t "QUICK t=1.0"
print "  [5/7] plot5_mpi_p2_quick.png"

# --- PLOT 6: MPI p=4 Upwind ---
set output "plot6_mpi_p4_upwind.png"
set title "{/=16 Q1(b) MPI p=4: Upwind — t = 0, 0.5, 1.0}"
plot \
  "mpi_p4_upwind_t0.0.csv" u 1:3 skip 1 w l lw 2 lc rgb C1 dt 2 t "Exact t=0", \
  "mpi_p4_upwind_t0.0.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C2 t "Upwind t=0", \
  "mpi_p4_upwind_t0.5.csv" u 1:3 skip 1 w l lw 2 lc rgb C3 dt 2 t "Exact t=0.5", \
  "mpi_p4_upwind_t0.5.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C4 t "Upwind t=0.5", \
  "mpi_p4_upwind_t1.0.csv" u 1:3 skip 1 w l lw 2 lc rgb CG dt 2 t "Exact t=1.0", \
  "mpi_p4_upwind_t1.0.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C5 t "Upwind t=1.0"
print "  [6/7] plot6_mpi_p4_upwind.png"

# --- PLOT 7: MPI p=4 QUICK ---
set output "plot7_mpi_p4_quick.png"
set title "{/=16 Q1(b) MPI p=4: QUICK — t = 0, 0.5, 1.0}"
plot \
  "mpi_p4_quick_t0.0.csv" u 1:3 skip 1 w l lw 2 lc rgb C1 dt 2 t "Exact t=0", \
  "mpi_p4_quick_t0.0.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C2 t "QUICK t=0", \
  "mpi_p4_quick_t0.5.csv" u 1:3 skip 1 w l lw 2 lc rgb C3 dt 2 t "Exact t=0.5", \
  "mpi_p4_quick_t0.5.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C4 t "QUICK t=0.5", \
  "mpi_p4_quick_t1.0.csv" u 1:3 skip 1 w l lw 2 lc rgb CG dt 2 t "Exact t=1.0", \
  "mpi_p4_quick_t1.0.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C5 t "QUICK t=1.0"
print "  [7/7] plot7_mpi_p4_quick.png"

print ""
print "Done. 7 plots generated."
