#!/bin/bash

# Roda o comando sinfo e usa awk para pegar o valor da coluna NODES
nodes=$(sinfo | awk 'NR==2{print $4}')
echo "> Número de nós disponíveis: $nodes"

# Recebe número de processos desejado
read -p "> Digite o número de nós desejados para executar: " NUM_NODES

# Verifica se NUM_NODES é maior do que nodes
if [ "$NUM_NODES" -le "$nodes" ]; then
    echo "> Número de processos solicitado ($NUM_NODES) está dentro do limite de nós disponíveis ($nodes). Prosseguindo..."
    
    # Recompila arquivos de execução
    echo "> Removendo antigos arquivos de compilaçãoe output:"
    make purge
    echo "> Compilando arquivos para execução:"
    make
    
    # Loop desde 1 até $NUM_NODES
    for (( process=1; process<=$NUM_NODES; process++ ))
    do
        # Rodar o programa para n processos MPI no mesmo host e medir o tempo da computação de knn
        echo "> Rodando sbatch --exclusive para 1 nodo com $process processo(s):"
        sbatch --output=knn_$process-processes_1-host.out --exclusive -N 1 ./scripts/knn-mpi-runner.sh $process
    done

    # Loop desde 1 até $NUM_NODES
    for (( nodo=1; nodo<=$NUM_NODES; nodo++ ))
    do
     # Rodar o programa para n processos MPI em n hosts diferentes e medir o tempo da computação de knn
        echo "> Rodando sbatch --exclusive para $nodo nodo(s) com 1 processo por nodo:"
        sbatch --output=knn_$nodo-processes_$nodo-hosts.out --nodes=$nodo --ntasks-per-node=1 --exclusive ./scripts/knn-mpi-runner.sh $nodo
    done
else
    echo "ERROR ==> O número de processos solicitado ($NUM_NODES) é maior do que o número de nós disponíveis ($nodes)."
    make purge
fi
