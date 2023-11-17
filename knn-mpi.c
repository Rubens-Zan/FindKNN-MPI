#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"
#include "max-heap.h"
#include "chrono.h"

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
        int ok = 0;

        for (int j = 0; j < length; j++)
        {
            if (computed_indices[i] == expected_indices[j])
                ok = 1;
        }

        if (!ok)
        {
            incorrect_count++;
        }
    }

    if (incorrect_count == 0)
    {
        printf("\n<<<Todos os resultados estão corretos.>>>\n");
    }
    else
    {
        printf("\n<<<%d discrepâncias encontradas.>>>\n", incorrect_count);
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
void knn(Point *local_Q, int local_nq, Point *P, int n, int D, int k, int *result_indices)
{
    pair_t neighbors[local_nq][k]; // matriz com nq linhas e k colunas
    pair_t inputTuple;
    Point query_point;

    // Em cada um dos buffers que esse processo MPI deve lidar
    // insere K vizinhos para inicializar a heap
    for (int i = 0; i < local_nq; i++)
    {
        int heapSize = 0;
        query_point = local_Q[i]; // pega a linha de Q

        // Calcula a distância de cada ponto em P até o ponto de consulta
        for (int j = 0; j < k; j++) // insere as K primeiras distancias euclideanas
        {
            inputTuple.val = j;
            inputTuple.key = euclidean_distance(&query_point, &P[j], D);
            insert((pair_t *)neighbors[i], &heapSize, inputTuple); // SEGFAULT AQUI
        }
    }

    int heapSize = k;

    // aplica decreaseMax para o restante dos elementos da matriz
    for (int i = 0; i < local_nq; i++)
    {
        Point query_point = local_Q[i];

        // Calcula a distância de cada ponto em P até o ponto de consulta
        for (int j = k; j < n; j++)
        {
            pair_t inputTuple;

            inputTuple.val = j;
            inputTuple.key = euclidean_distance(&query_point, &P[j], D);

            decreaseMax((pair_t *)neighbors[i], heapSize, inputTuple);
        }
    }

    // Armazena os índices dos k vizinhos mais próximos
    for (int i = 0; i < local_nq; i++)
        for (int m = 0; m < k; m++)
            result_indices[i * k + m] = neighbors[i][m].val;
}

int main(int argc, char *argv[])
{
    int rank, size;

    if (argc != 5)
    {
        printf("Expected: mpirun -np <num_processos> knn-mpi <nq> <npp> <d> <k> %d \n", argc);
        exit(1);
    }

    int nq = atoi(argv[1]); // Total de pontos em Q
    int n = atoi(argv[2]);  // Total de pontos em P
    int D = atoi(argv[3]);  // Número de dimensões dos pontos
    int k = atoi(argv[4]);  // Número de vizinhos mais próximos

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    chronometer_t chrono;

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
        result_indices = (int *)malloc(nq * k * sizeof(int));
        // local_Q é todo Q no processo 0
        // local_Q = (Point *)malloc(nq * sizeof(Point));

        // Inicializa a semente do gerador de números aleatórios
        srand(time(NULL));
        // Gera pontos aleatórios
        generate_random_points(P, n, D);
        generate_random_points(Q, nq, D);

        chrono_reset(&chrono);
        chrono_start(&chrono);
    }

    /****************************** Distribuição de Q entre os processos ******************************/
    // Determina o número de pontos de Q que cada processo irá receber
    int local_nq = nq / size;

    // Cada processo terá um subconjunto de Q para processar
    local_Q = (Point *)malloc(local_nq * sizeof(Point));

    // A matriz result_indices é alocada em cada processo para armazenar os resultados locais
    int *local_result_indices = (int *)malloc(local_nq * k * sizeof(int));

    // O processo com rank 0 distribui os pontos de Q para todos os processos
    MPI_Scatter(Q,
                local_nq * sizeof(Point),
                MPI_BYTE,
                local_Q,
                local_nq * sizeof(Point),
                MPI_BYTE,
                0,
                MPI_COMM_WORLD);

    // Recebe o nome do processador
    int name_len;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    MPI_Get_processor_name(processor_name, &name_len);

    // Print informações do nodo
    printf("Host %s has rank %d out of %d MPI processes\n", processor_name, rank, size);
    /****************************** Distribuição de Q entre os processos ******************************/

    /****************************** Execução do KNN no subconjunto de Q para cada processo *****************************/
    // Broadcast do conjunto de pontos P para todos os n processos
    MPI_Bcast(P,
              n * sizeof(Point),
              MPI_BYTE,
              0,
              MPI_COMM_WORLD);
    knn(local_Q, local_nq, P, n, D, k, local_result_indices);
    /****************************** Execução do KNN no subconjunto de Q para cada processo *****************************/

    /****************************** Reunião dos resultados dos vizinhos mais próximos em rank 0 ******************************/
    MPI_Gather(local_result_indices,
               local_nq * k,
               MPI_INT,
               result_indices,
               local_nq * k,
               MPI_INT,
               0,
               MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    /****************************** Reunião dos resultados dos vizinhos mais próximos em rank 0 ******************************/

    /****************************** Verificação dos Resultados ******************************/
    if (rank == 0)
    {
        chrono_stop(&chrono);
        chrono_reportTime(&chrono, "chrono");

        // Calcula e imprimi a VAZAO (nesse caso: numero de BYTES/s)
        double total_time_in_seconds = (double)chrono_gettotal(&chrono) /
                                       ((double)1000 * 1000 * 1000);
        printf("total_time_in_seconds: %lf s\n", total_time_in_seconds);
    }
#ifdef DEBUG
    // Generate expected results on rank 0 for verification purposes
    int *expected_results = NULL;
    if (rank == 0)
    {
        double GFLOPS = (((double)nq * k * n) / ((double)total_time_in_seconds * 1000 * 1000 * 1000));
        printf("Throughput: %lf GFLOPS\n", GFLOPS * (size - 1));

        // Array 'expected_results' com os índices esperados
        expected_results = (int *)malloc(nq * k * sizeof(int));
        generate_expected_results(Q, nq, P, n, D, k, expected_results);

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

    if (rank == 0)
    {
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

        verify_results(result_indices, expected_results, nq * k);
    }
#endif
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
