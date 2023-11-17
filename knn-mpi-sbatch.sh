#!/bin/bash

# Roda o comando sinfo e usa awk para pegar o valor da coluna NODES
nodes=$(sinfo | awk 'NR==2{print $4}')

# Agora a variável $nodes tem o valor do campo NODES
echo "Número de nós disponíveis: $nodes"

rm slurm*
make purge && make

# Rodar o programa para APENAS 1 processo MPI e medir o tempo da computaçao de knn
echo "Rodando sbatch --exclusive para 1 nodo..."
# Iniciar a medição de tempo
start_time=$(date +%s)
sbatch --exclusive -N 1 run-knn-mpi 1 
end_time=$(date +%s)
echo "Tempo total de execução: $((end_time - start_time)) segundos"


# Rodar o programa para 4 processos MPI no mesmo host e medir o tempo da computaçao de knn
echo "Rodando sbatch --exclusive para 1 nodo..."
# Iniciar a medição de tempo
start_time=$(date +%s)
sbatch --exclusive -N 1 run-knn-mpi 4 
end_time=$(date +%s)
echo "Tempo total de execução: $((end_time - start_time)) segundos"

# Rodar o programa para 4 processos MPI em hosts diferentes e medir o tempo da computaçao de knn
echo "Rodando sbatch --exclusive para 4 nodos..."
# Iniciar a medição de tempo
start_time=$(date +%s)
sbatch ./run-knn-mpi 4
end_time=$(date +%s)
echo "Tempo total de execução: $((end_time - start_time)) segundos"
