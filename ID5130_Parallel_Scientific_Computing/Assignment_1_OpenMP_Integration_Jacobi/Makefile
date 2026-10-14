# Makefile for ID5130 Assignment 1
# Parallel Scientific Computing

CC = gcc
CFLAGS = -O3 -Wall
OMP_FLAGS = -fopenmp
LIBS = -lm

all: q1a_riemann q1b_montecarlo q2_jacobi

q1a_riemann: q1a_riemann.c
	$(CC) $(CFLAGS) $(OMP_FLAGS) -o q1a_riemann q1a_riemann.c $(LIBS)

q1b_montecarlo: q1b_montecarlo.c
	$(CC) $(CFLAGS) $(OMP_FLAGS) -o q1b_montecarlo q1b_montecarlo.c $(LIBS)

q2_jacobi: q2_jacobi.c
	$(CC) $(CFLAGS) $(OMP_FLAGS) -o q2_jacobi q2_jacobi.c $(LIBS)

clean:
	rm -f q1a_riemann q1b_montecarlo q2_jacobi *.csv

.PHONY: all clean
