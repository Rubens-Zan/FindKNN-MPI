#!/bin/bash

# Roda o comando sinfo e usa awk para pegar o valor da coluna NODES
nodes=$(sinfo | awk 'NR==2{print $4}')

# Agora a variável $nodes tem o valor do campo NODES
echo "Número de nós disponíveis: $nodes"

# Rodar o programa para APENAS 1 processo MPI e medir o tempo da computaçao de knn
mpirun -N 1 -np 1 knn-mpi  128 400000 300 1024

# Rodar o programa para 4 processos MPI no mesmo host e medir o tempo da computaçao de knn
mpirun -N 1 -np 4 knn-mpi  128 400000 300 1024

# Rodar o programa para 4 processos MPI em hosts diferentes e medir o tempo da computaçao de knn
mpirun -N 1 -np 4 knn-mpi  128 400000 300 1024
