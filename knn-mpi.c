#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"
#include "max-heap.h"

#define MAX_POINTS 300
// Definição da estrutura para um ponto com D dimensões
typedef struct
{
    float coords[MAX_POINTS]; // Número fixo de dimensões para simplificar
} Point;

// Função para gerar pontos aleatórios
void generate_random_points(Point *points, int n_points, int dimensions)
{
    for (int i = 0; i < n_points; i++)
    {
        for (int j = 0; j < dimensions; j++)
        {
            points[i].coords[j] = (float)rand() / RAND_MAX; // Valores aleatórios entre 0 e 1
        }
    }
}

// Função para calcular a distância euclidiana entre dois pontos
float euclidean_distance(const Point *a, const Point *b, int dimensions)
{
    float distance = 0.0;
    for (int i = 0; i < dimensions; i++)
    {
        float diff = a->coords[i] - b->coords[i];
        distance += diff * diff;
    }
    
    return (distance);
}

// Função de comparação usada pelo qsort
int compare_pair(const void *a, const void *b)
{
    float diff = ((pair_t *)a)->key - ((pair_t *)b)->key;
    return (diff >= 0) - (diff < 0);
}

// Função para comparar os índices dos vizinhos mais próximos gerados com os esperados
void verify_results(int *computed_indices, int *expected_indices, int length)
{
    int incorrect_count = 0;
    for (int i = 0; i < length; i++)
    {
        if (computed_indices[i] != expected_indices[i])
        {
            printf("Índice %d: calculado=%d, esperado=%d\n", i, computed_indices[i], expected_indices[i]);
            incorrect_count++;
        }
    }
    if (incorrect_count == 0)
    {
        printf("Todos os resultados estão corretos.\n");
    }
    else
    {
        printf("%d discrepâncias encontradas.\n", incorrect_count);
    }
}

// Função Auxiliar para gerar os resultados esperados
void generate_expected_results(Point *Q, int nq, Point *P, int n, int D, int k, int *expected_results)
{
    pair_t *neighbors = (pair_t *)malloc(n * sizeof(pair_t));

    // Executa o KNN sequencialmente para cada ponto de consulta em Q
    for (int i = 0; i < nq; i++)
    {
        Point query_point = Q[i];

        // Calcula a distância de cada ponto em P até o ponto de consulta
        for (int j = 0; j < n; j++)
        {
            neighbors[j].val = j;
            neighbors[j].key = euclidean_distance(&query_point, &P[j], D);
        }

        // Ordena todos os vizinhos pela distância usando um algoritmo de ordenação
        qsort(neighbors, n, sizeof(pair_t), compare_pair);

        // Armazena os índices dos k vizinhos mais próximos em expected_results
        for (int m = 0; m < k; m++)
        {
            expected_results[i * k + m] = neighbors[m].val;
        }
    }

    free(neighbors);
}

// Execução do KNN no subconjunto de Q para cada processo
void knn(Point *local_Q, int local_nq, Point *P, int n, int D, int k, int *result_indices, int rank)
{
    pair_t buff[k];
    pair_t neighbors[local_nq][k]; // VERIFICAR A ALOCACAO
    

    int meuRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &meuRank);
    pair_t inputTuple; 
    Point query_point;

    for (int i = 0; i < local_nq; i++)
    {
        int heapSize = 0;
        query_point = local_Q[i]; // pega a linha de Q

        // Calcula a distância de cada ponto em P até o ponto de consulta
        for (int j =  0; j < k; j++) // insere as K primeiras distancias euclideanas
        {
            inputTuple.val = j;
            inputTuple.key = euclidean_distance(&query_point, &P[j], D);
            insert( (pair_t *)neighbors[i], &heapSize, inputTuple );     // SEGFAULT AQUI
        }
    }

    int heapSize = k;

    for (int i = 0; i < local_nq; i++)
    {
        Point query_point = local_Q[i];

        // Calcula a distância de cada ponto em P até o ponto de consulta
        for (int j =  k; j < n; j++)
        {
            pair_t inputTuple; 

            inputTuple.val = j;
            inputTuple.key = euclidean_distance(&query_point, &P[j], D);
            // Ordena os vizinhos pela distância
            decreaseMax((pair_t *)neighbors[i], heapSize, inputTuple);
        }
    }
    // Armazena os índices dos k vizinhos mais próximos
    // if (meuRank == 0){
        // for (int i = 0; i < local_nq; i++)
            // drawHeapTree( neighbors[i], heapSize , k );

    //     for (int i = 0; i < local_nq; i++){
    //             printf("\n Para a linha: %d \n",i); 
    //             for (int j =  0; j < k; j++) // insere as K primeiras distancias euclideanas
    //                 printf("(%d,%d) =  %f %d \n", i,j,neighbors[i][j].key,neighbors[i][j].val );
    // }
    

}

int main(int argc, char *argv[])
{
    int rank, size;
    int nq = 8; // Total de pontos em Q
    int n = 20; // Total de pontos em P
    int D = 10; // Número de dimensões dos pontos
    int k = 5;  // Número de vizinhos mais próximos
    
    if (argc != 5){
        printf("Expected: mpirun -np 4 knn-mpi <nq> <npp> <d> <k> %d \n", argc);
        exit(1); 
    }

    nq = atoi(argv[1]);
    n  = atoi(argv[2]);
    D = atoi(argv[3]);
    k = atoi(argv[4]);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    Point *P = NULL;
    Point *Q = NULL;

    Point *local_Q = NULL;      // Cada processo terá um subconjunto de Q para processar
    int *result_indices = NULL; // Esta matriz armazenará os índices dos k vizinhos mais próximos

    // O processo com rank 0 gera os conjuntos de pontos P e Q
    P = (Point *)malloc(n * sizeof(Point));
    
    if (rank == 0)
    {
        // Alocação de memória
        Q = (Point *)malloc(nq * sizeof(Point));
        // local_Q = (Point *)malloc(nq * sizeof(Point)); // local_Q é todo Q no processo 0
        result_indices = (int *)malloc(nq * k * sizeof(int));

        // Inicializa a semente do gerador de números aleatórios
        // srand(time(NULL));
        srand(42); // Fixed seed for reproducibility
        generate_random_points(P, n, D);
        generate_random_points(Q, nq, D);
    }

    // Generate expected results on rank 0 for verification purposes
    int *expected_results = NULL;
    if (rank == 0)
    {
        expected_results = (int *)malloc(nq * k * sizeof(int));
        generate_expected_results(Q, nq, P, n, D, k, expected_results);
    }

    /****************************** Distribuição de Q entre os processos ******************************/
    // Determina o número de pontos de Q que cada processo irá receber
    int local_nq = nq / size;

    // Cada processo terá um subconjunto de Q para processar
    local_Q = (Point *)malloc(local_nq * sizeof(Point));

    // A matriz result_indices é alocada em cada processo para armazenar os resultados locais
    int *local_result_indices = (int *)malloc(local_nq * k * sizeof(int));

    // O processo com rank 0 distribui os pontos de Q para todos os processos
    MPI_Scatter(Q, local_nq * sizeof(Point), MPI_BYTE,
                local_Q, local_nq * sizeof(Point), MPI_BYTE,
                0, MPI_COMM_WORLD);
    /****************************** Distribuição de Q entre os processos ******************************/

    /****************************** Execução do KNN no subconjunto de Q para cada processo *****************************/
 
    MPI_Bcast(P, n * sizeof(Point), MPI_BYTE, 0, MPI_COMM_WORLD);

    knn(local_Q, local_nq, P, n, D, k, local_result_indices, rank);
    /****************************** Execução do KNN no subconjunto de Q para cada processo *****************************/

    /****************************** Reunião dos resultados dos vizinhos mais próximos em rank 0 ******************************/
    // O processo com rank 0 precisa ter memória suficiente para receber todos os resultados
    if (rank == 0)
    {
        result_indices = (int *)malloc(nq * k * sizeof(int));
    }

    // Reunião dos resultados dos vizinhos mais próximos em rank 0
    MPI_Gather(local_result_indices, local_nq * k, MPI_INT,
               result_indices, local_nq * k, MPI_INT,
               0, MPI_COMM_WORLD);
    /****************************** Reunião dos resultados dos vizinhos mais próximos em rank 0 ******************************/

    /****************************** Verificação dos Resultados ******************************/

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0)
    {
        // Array 'expected_results' com os índices esperados
        // verify_results(result_indices, expected_results, nq * k);
        #ifdef DEBUG 
        // Imprimir os resultados gerados pelo KNN
        printf("\nÍndices dos vizinhos mais próximos calculados pelo KNN:\n");
        for (int i = 0; i < nq; i++)
        {
            printf("Ponto de consulta %d: ", i);
            for (int j = 0; j < k; j++)
            {
                printf("%d ", result_indices[i * k + j]);
            }
            printf("\n");
        }

        // Imprimir os resultados esperados pelo KNN
        printf("\nÍndices dos vizinhos mais próximos esperados pelo KNN:\n");
        for (int i = 0; i < nq; i++)
        {
            printf("Ponto de consulta %d: ", i);
            for (int j = 0; j < k; j++)
            {
                printf("%d ", expected_results[i * k + j]);
            }
            printf("\n");
        }
        #endif

        printf("\nMatriz P:\n");
        for (int i = 0; i < n; i++)
        {
            printf("Ponto %d: ", i);
            for (int j = 0; j < D; j++)
            {
                printf("%.2f ", P[i].coords[j]);
                if (j < D - 1)
                {
                    printf(", ");
                }
            }
            printf("\n");
        }

        printf("\nMatriz Q:\n");
        for (int i = 0; i < nq; i++)
        {
            printf("Ponto %d: ", i);
            for (int j = 0; j < D; j++)
            {
                printf("%.2f ", Q[i].coords[j]);
                if (j < D - 1)
                {
                    printf(", ");
                }
            }
            printf("\n");
        }
    }

    sleep(1);
    printf("\nMatriz local_Q do rank %d local nq: %d:\n", rank, local_nq);
    for (int i = 0; i < nq / size; i++)
    {
        printf("Ponto %d: ", i);
        for (int j = 0; j < D; j++)
        {
            printf("%.2f ", local_Q[i].coords[j]);
            if (j < D - 1)
            {
                printf(", ");
            }
        }
        printf("\n");
    }
    /****************************** Verificação dos Resultados ******************************/

    /****************************** Finalização ******************************/
    // Limpeza de memória
    free(local_Q);
    free(local_result_indices);
    if (rank != 0)
    {
        free(P);
    }

    MPI_Finalize();
    return 0;
}
