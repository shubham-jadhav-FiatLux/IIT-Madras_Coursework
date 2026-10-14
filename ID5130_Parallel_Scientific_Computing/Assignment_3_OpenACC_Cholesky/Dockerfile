FROM nvcr.io/nvidia/nvhpc:22.11-devel-cuda_multi-ubuntu20.04

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    gnuplot \
    tzdata \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
