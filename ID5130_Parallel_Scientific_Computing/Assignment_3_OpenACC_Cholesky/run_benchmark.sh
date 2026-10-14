#!/bin/bash

DOCKER_DEVICES="--device /dev/nvidia0 --device /dev/nvidia-uvm --device /dev/nvidia-uvm-tools --device /dev/nvidiactl"
DRIVER_LIBS="-v /usr/lib/libcuda.so.470.256.02:/usr/lib/x86_64-linux-gnu/libcuda.so.1:ro \
             -v /usr/lib/libnvidia-ml.so.470.256.02:/usr/lib/x86_64-linux-gnu/libnvidia-ml.so.1:ro"

echo "Building Docker image..."
docker build -t cholesky-nvhpc .

echo "Running benchmark inside Docker (GPU: MX130 cc50)..."
docker run --rm $DOCKER_DEVICES $DRIVER_LIBS -v "$(pwd):/workspace" cholesky-nvhpc bash -c "
    cd /workspace

    echo '=== Compiling V1 (Column-Oriented) ==='
    nvc -acc -gpu=cc50 -O3 cholesky_v1.c -o cholesky_v1_gpu -lm

    echo '=== Compiling V2 (Blocked/Tiled) ==='
    nvc -acc -gpu=cc50 -O3 cholesky_v2.c -o cholesky_v2_gpu -lm

    echo ''
    echo '=== Running V1 ==='
    ./cholesky_v1_gpu

    echo '=== Running V2 ==='
    ./cholesky_v2_gpu

    echo 'Generating combined timing CSV for plot...'
    python3 -c \"
import csv
v1 = {}
v2 = {}
with open('cholesky_v1_timing.csv') as f:
    for row in csv.DictReader(f):
        v1[row['N']] = row['v1_time']
with open('cholesky_v2_timing.csv') as f:
    for row in csv.DictReader(f):
        v2[row['N']] = row['v2_time']
with open('cholesky_v2_timing.csv') as f:
    rows = list(csv.DictReader(f))
with open('cholesky_timing.csv', 'w', newline='') as f:
    w = csv.writer(f)
    w.writerow(['N','serial_time','v1_time','v2_time'])
    for row in rows:
        n = row['N']
        w.writerow([n, row['serial_time'], v1.get(n,''), row['v2_time']])
\"

    echo 'Plotting...'
    gnuplot plot_cholesky.gp
    rm -f cholesky_v1_gpu cholesky_v2_gpu
"

echo "Done. Outputs: cholesky_timing.csv, plot_cholesky_timing.png"
