CC = gcc
MPICC = mpicc
CFLAGS = -O3 -Wall
LIBS = -lm

all: q1_serial q1_mpi q2_serial q2_mpi

q1_serial: q1_serial.c
	$(CC) $(CFLAGS) -o q1_serial q1_serial.c $(LIBS)

q1_mpi: q1_mpi.c
	$(MPICC) $(CFLAGS) -o q1_mpi q1_mpi.c $(LIBS)

q2_serial: q2_serial.c
	$(CC) $(CFLAGS) -o q2_serial q2_serial.c $(LIBS)

q2_mpi: q2_mpi.c
	$(MPICC) $(CFLAGS) -o q2_mpi q2_mpi.c $(LIBS)

clean:
	rm -f q1_serial q1_mpi q2_serial q2_mpi *.csv

.PHONY: all clean
