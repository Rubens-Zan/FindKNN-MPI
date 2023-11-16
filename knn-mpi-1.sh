#!/bin/bash

echo "Executando knn-mpi para $1 processos em $2 nodos."
mpirun -N $2 -np $1 knn-mpi  128 400000 300 1024