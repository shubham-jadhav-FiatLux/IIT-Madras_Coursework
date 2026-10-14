#!/usr/bin/gnuplot
# ID5130 Assignment 2 Q2 — All Plots

set terminal pngcairo enhanced size 1000,700 font "Helvetica,14" lw 1.5
set datafile separator ","

C1 = "#2166AC"
C2 = "#D6604D"
C3 = "#2CA02C"
C4 = "#984EA3"
CG = "#888888"

set style line 100 lt 1 lc rgb "#E8E8E8" lw 0.4
set grid back ls 100
set border 3 lw 1.2
set tics nomirror

# --- PLOT 1: Q2(a) phi vs x at y=0 (delta=0.1) ---
set output "plot_q2a_phi_vs_x.png"
set xlabel "x"
set ylabel "{/Symbol f}(x, y=0)"
set title "{/=16 Q2(a) Serial: {/Symbol f} vs x at y = 0  ({/Symbol D} = 0.1)}"
set key top left spacing 1.3
plot "serial_phi_vs_x_y0_d0.1000.csv" u 1:2 skip 1 w linespoints pt 7 ps 0.8 lw 2 lc rgb C1 t "{/Symbol f}(x, 0)"
print "  [1/6] plot_q2a_phi_vs_x.png"

# --- PLOT 2: Q2(a) phi vs y at x=0 (delta=0.1) ---
set output "plot_q2a_phi_vs_y.png"
set xlabel "y"
set ylabel "{/Symbol f}(x=0, y)"
set title "{/=16 Q2(a) Serial: {/Symbol f} vs y at x = 0  ({/Symbol D} = 0.1)}"
plot "serial_phi_vs_y_x0_d0.1000.csv" u 1:2 skip 1 w linespoints pt 7 ps 0.8 lw 2 lc rgb C1 t "{/Symbol f}(0, y)"
print "  [2/6] plot_q2a_phi_vs_y.png"

# --- PLOT 3: Q2(b) phi vs x at y=0 (delta=0.01), serial + MPI ---
set output "plot_q2b_phi_vs_x.png"
set xlabel "x"
set ylabel "{/Symbol f}(x, y=0)"
set title "{/=16 Q2(b): {/Symbol f} vs x at y = 0  ({/Symbol D} = 0.01)}"
set key top left spacing 1.2
plot \
  "serial_phi_vs_x_y0_d0.0100.csv" u 1:2 skip 1 w l lw 2.5 lc rgb C1 t "Serial", \
  "mpi_p2_phi_vs_x_y0_d0.0100.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C2 dt 2 t "MPI p=2", \
  "mpi_p4_phi_vs_x_y0_d0.0100.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C3 dt 4 t "MPI p=4", \
  "mpi_p8_phi_vs_x_y0_d0.0100.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C4 dt 6 t "MPI p=8"
print "  [3/6] plot_q2b_phi_vs_x.png"

# --- PLOT 4: Q2(b) phi vs y at x=0 (delta=0.01), serial + MPI ---
set output "plot_q2b_phi_vs_y.png"
set xlabel "y"
set ylabel "{/Symbol f}(x=0, y)"
set title "{/=16 Q2(b): {/Symbol f} vs y at x = 0  ({/Symbol D} = 0.01)}"
plot \
  "serial_phi_vs_y_x0_d0.0100.csv" u 1:2 skip 1 w l lw 2.5 lc rgb C1 t "Serial", \
  "mpi_p2_phi_vs_y_x0_d0.0100.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C2 dt 2 t "MPI p=2", \
  "mpi_p4_phi_vs_y_x0_d0.0100.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C3 dt 4 t "MPI p=4", \
  "mpi_p8_phi_vs_y_x0_d0.0100.csv" u 1:2 skip 1 w l lw 1.5 lc rgb C4 dt 6 t "MPI p=8"
print "  [4/6] plot_q2b_phi_vs_y.png"

# --- PLOT 5: Q2(a) phi vs x at y=0 (delta=0.1 vs 0.01 comparison) ---
set output "plot_q2_grid_compare_x.png"
set xlabel "x"
set ylabel "{/Symbol f}(x, y=0)"
set title "{/=16 Q2: Grid refinement — {/Symbol f} vs x at y = 0}"
plot \
  "serial_phi_vs_x_y0_d0.1000.csv" u 1:2 skip 1 w linespoints pt 6 ps 1.0 lw 1.5 lc rgb C2 t "{/Symbol D}=0.1 (21x21)", \
  "serial_phi_vs_x_y0_d0.0100.csv" u 1:2 skip 1 w l lw 2 lc rgb C1 t "{/Symbol D}=0.01 (201x201)"
print "  [5/6] plot_q2_grid_compare_x.png"

# --- PLOT 6: Q2(a) phi vs y at x=0 (delta=0.1 vs 0.01 comparison) ---
set output "plot_q2_grid_compare_y.png"
set xlabel "y"
set ylabel "{/Symbol f}(x=0, y)"
set title "{/=16 Q2: Grid refinement — {/Symbol f} vs y at x = 0}"
plot \
  "serial_phi_vs_y_x0_d0.1000.csv" u 1:2 skip 1 w linespoints pt 6 ps 1.0 lw 1.5 lc rgb C2 t "{/Symbol D}=0.1 (21x21)", \
  "serial_phi_vs_y_x0_d0.0100.csv" u 1:2 skip 1 w l lw 2 lc rgb C1 t "{/Symbol D}=0.01 (201x201)"
print "  [6/6] plot_q2_grid_compare_y.png"

print ""
print "Done. 6 Q2 plots generated."
