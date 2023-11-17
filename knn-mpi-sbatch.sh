#!/bin/bash

# Roda o comando sinfo e usa awk para pegar o valor da coluna NODES
nodes=$(sinfo | awk 'NR==2{print $4}')

# Agora a variável $nodes tem o valor do campo NODES
echo "Número de nós disponíveis: $nodes"

rm slurm*
make purge && make

# Rodar o programa para APENAS 1 processo MPI e medir o tempo da computaçao de knn
echo "Rodando sbatch --exclusive para 1 nodo..."
sbatch --exclusive -N run-knn-mpi.sh 1 

# Rodar o programa para 4 processos MPI no mesmo host e medir o tempo da computaçao de knn
echo "Rodando sbatch --exclusive para 1 nodo..."
sbatch --exclusive -N run-knn-mpi.sh 4 

# Rodar o programa para 4 processos MPI em hosts diferentes e medir o tempo da computaçao de knn
echo "Rodando sbatch --exclusive para 4 nodos..."
sbatch run-knn-mpi.sh 4
