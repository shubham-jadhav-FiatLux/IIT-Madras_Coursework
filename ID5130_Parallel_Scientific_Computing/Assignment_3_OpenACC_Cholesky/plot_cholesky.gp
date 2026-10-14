set terminal pngcairo enhanced font "Arial,12" size 1000,700
set output "plot_cholesky_timing.png"

set title "Cholesky Decomposition: Serial vs OpenACC V1 (Column) vs V2 (Blocked)" font ",14"
set xlabel "Matrix Size N"
set ylabel "Time (seconds)"
set grid
set key left top box
set logscale y
set logscale x

set datafile separator ","

plot "cholesky_timing.csv" using 1:2 with linespoints lw 2 pt 7 ps 1.3 lc rgb "#2196F3" title "Serial", \
     "cholesky_timing.csv" using 1:3 with linespoints lw 2 pt 5 ps 1.3 lc rgb "#FF9800" title "OpenACC V1 (Column)", \
     "cholesky_timing.csv" using 1:4 with linespoints lw 2 pt 9 ps 1.3 lc rgb "#4CAF50" title "OpenACC V2 (Blocked/Tiled)"
