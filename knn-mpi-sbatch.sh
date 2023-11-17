#!/bin/bash

# Roda o comando sinfo e usa awk para pegar o valor da coluna NODES
nodes=$(sinfo | awk 'NR==2{print $4}')

# Agora a variável $nodes tem o valor do campo NODES
echo "> Número de nós disponíveis: $nodes"

# Recebe número de processos desejado
NUM_NODES=$1

# Verifica se NUM_NODES é maior do que nodes
if [ "$NUM_NODES" -le "$nodes" ]; then
    echo "> Número de processos solicitado ($NUM_NODES) está dentro do limite de nós disponíveis ($nodes). Prosseguindo..."
    
    # Verifica se existem arquivos com padrão slurm* no diretório atual
    if ls slurm* 1> /dev/null 2>&1; then
        echo "> Removendo arquivos slurm antigos."
        rm slurm*
    else
        echo "> Nenhum arquivo slurm antigo para remover. Prosseguindo...."
    fi
    
    # Recompila arquivos de execução
    echo "> Removendo antigos arquivos de compilação:"
    make purge
    echo "> Compilando arquivos para execução:"
    make
    
    # Rodar o programa para APENAS 1 processo MPI e medir o tempo da computaçao de knn
    echo "> Rodando sbatch --exclusive para 1 nodo com 1 processo:"
    sbatch --exclusive -N 1 knn-mpi-runner.sh 1
    
    
    # Loop desde 1 até $NUM_NODES
    for (( process=1; process<=$NUM_NODES; process++ ))
    do
        # Rodar o programa para n processos MPI no mesmo host e medir o tempo da computação de knn
        echo "> Rodando sbatch --exclusive para 1 nodo com $process processo(s):"
        sbatch --exclusive -N 1 knn-mpi-runner.sh $process
    done

    # Loop desde 1 até $NUM_NODES
    for (( nodo=1; nodo<=$NUM_NODES; nodo++ ))
    do
     # Rodar o programa para n processos MPI em n hosts diferentes e medir o tempo da computação de knn
        echo "> Rodando sbatch --exclusive para $nodo nodo(s) com 1 processo por nodo:"
        sbatch --nodes=$nodo --ntasks-per-node=1 --exclusive knn-mpi-runner.sh $nodo
    done
    
    # # Rodar o programa para 4 processos MPI no mesmo host e medir o tempo da computaçao de knn
    # echo "> Rodando sbatch --exclusive para 1 nodo com $NUM_NODES processos:"
    # sbatch --exclusive -N 1 knn-mpi-runner.sh $NUM_NODES
    
    # # Rodar o programa para 4 processos MPI em hosts diferentes e medir o tempo da computaçao de knn
    # echo "> Rodando sbatch --exclusive para $NUM_NODES nodos com 1 processo por nodo:"
    # sbatch --nodes=$NUM_NODES --ntasks-per-node=1 --exclusive knn-mpi-runner.sh $NUM_NODES
else
    echo "ERROR: O número de processos solicitado ($NUM_NODES) é maior do que o número de nós disponíveis ($nodes)."
    make purge
fi
