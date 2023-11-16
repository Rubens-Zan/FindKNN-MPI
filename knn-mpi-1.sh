#!/bin/bash

# Rodar o programa para APENAS 1 processo MPI e medir o tempo da computaçao de knn
mpirun -np 1 knn-mpi  128 400000 300 1024