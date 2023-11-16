#!/bin/bash

echo "Executando knn-mpi para $1 processos."
mpirun -np $1 knn-mpi  128 400000 300 1024