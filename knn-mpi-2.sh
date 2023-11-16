#!/bin/bash

# Rodar o programa para 4 processos MPI no mesmo host e medir o tempo da computaçao de knn
mpirun -np 4 knn-mpi  128 400000 300 1024