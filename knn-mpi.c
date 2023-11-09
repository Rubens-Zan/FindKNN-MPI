#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"
#include "max-heap.h"

// Definição da estrutura para um ponto com D dimensões
typedef struct
{
    float coords[400000]; // Número fixo de dimensões para simplificar
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

// Protótipo da função KNN (a ser implementada)
void knn(Point *Q, int nq, Point *P, int n, int D, int k, int *result_indices);

void verificaKNN(float *Q, int nq, float *P, int n, int D, int k, int *R)
{
    // note que R tem nq linhas por k colunas, para qualquer tamanho de k (colunas)
    // entao é linearizado para acesso como um VETOR
    printf(" ------------ VERIFICA KNN --------------- ");
    for (int linha = 0; linha < nq; linha++)
    {
        printf("knn[%d]: ", linha);
        for (int coluna = 0; coluna < k; coluna++)
            printf("%d ", R[linha * k + coluna]);
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    int rank, size;

    int n;  // Total de pontos em P
    int nq; // Total de pontos em Q
    int D;  // Número de dimensões dos pontos
    int k;  // Número de vizinhos mais próximos

    if (argc != 5)
    {
        printf("usage: %s <nq> <npp> <d> <k>\n",
               argv[0]);
        return 0;
    }
    else
    {
        nq = atoi(argv[1]);
        n = atoi(argv[2]);
        D = atoi(argv[3]);
        k = atoi(argv[4]);
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    Point *P = NULL;            // P = Pontos da base (dataset) (matriz de floats)
    Point *Q = NULL;            // Q = Pontos de consulta, send |Q| o tamanho de Q. (matriz de floats)
    Point *local_Q = NULL;      // Cada processo terá um subconjunto de Q para processar
    int *result_indices = NULL; // Esta matriz armazenará os índices dos k vizinhos mais próximos

    // O processo com rank 0 gera os conjuntos de pontos P e Q
    if (rank == 0)
    {
        // Alocação de memória
        P = (Point *)malloc(n * sizeof(Point));
        Q = (Point *)malloc(nq * sizeof(Point));
        // local_Q é todo Q no processo 0
        local_Q = (Point *)malloc(nq * sizeof(Point)); 
        result_indices = (int *)malloc(nq * k * sizeof(int));

        // Inicializa a semente do gerador de números aleatórios
        srand(time(NULL));
        generate_random_points(P, n, D);
        generate_random_points(local_Q, nq, D);
    }

     /********************** Distribuição de Q entre os processos *********************/ 
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
    /********************** Distribuição de Q entre os processos *********************/ 

    /********************** Execução do KNN no subconjunto de Q para cada processo *********************/ 
    // Se o conjunto P é grande, você pode querer enviar os dados de P
    // para cada processo de forma mais controlada. No entanto, para simplificar,
    // estamos assumindo que todo P é enviado para todos os processos.
    if (rank != 0)
    {
        P = (Point *)malloc(n * sizeof(Point));
    }
    MPI_Bcast(P, n * sizeof(Point), MPI_BYTE, 0, MPI_COMM_WORLD);

    knn(local_Q, local_nq, P, n, D, k, local_result_indices);
    /********************** Execução do KNN no subconjunto de Q para cada processo *********************/ 

    /********************** Reunião dos resultados dos vizinhos mais próximos em rank 0 *********************/ 
    // O processo com rank 0 precisa ter memória suficiente para receber todos os resultados
    if (rank == 0)
    {
        result_indices = (int *)malloc(nq * k * sizeof(int));
    }

    // Reunião dos resultados dos vizinhos mais próximos em rank 0
    MPI_Gather(local_result_indices, local_nq * k, MPI_INT,
               result_indices, local_nq * k, MPI_INT,
               0, MPI_COMM_WORLD);

    // O processo com rank 0 agora tem todos os índices dos k vizinhos mais próximos
    // de todos os pontos de consulta em Q
    if (rank == 0)
    {
        printf("sou o 0 \n");
        // Aqui, result_indices contém os resultados de todos os processos
        // Processamento adicional ou saída dos resultados pode ser feito aqui

        for (int linha = 0; linha < nq; linha++)
        {
            printf("knn[%d]: ", linha);
            for (int coluna = 0; coluna < k; coluna++)
                printf("%d ", result_indices[linha * k + coluna]);
            printf("\n");
        }
    }
    /********************** Reunião dos resultados dos vizinhos mais próximos em rank 0 *********************/ 

    // Limpeza de memória
    free(local_Q);
    free(local_result_indices);
    if (rank != 0)
    {
        free(P);
    }

    // Finalização MPI
    MPI_Finalize();
    return 0;
}

// // Função auxiliar para comparar dois vizinhos (usada com qsort)
// int compare_neighbors(const void *a, const void *b) {
//     float dist_a = ((Neighbor *)a)->distance;
//     float dist_b = ((Neighbor *)b)->distance;
//     return (dist_a > dist_b) - (dist_a < dist_b);
// }

// Execução do KNN no subconjunto de Q para cada processo
void knn(Point *local_Q, int local_nq, Point *P, int n, int D, int k, int *result_indices)
{
    pair_t *neighbors = (pair_t *)malloc(n * sizeof(pair_t));

    for (int i = 0; i < local_nq; i++)
    {
        Point query_point = local_Q[i];

        // Calcula a distância de cada ponto em P até o ponto de consulta
        for (int j = 0; j < n; j++)
        {
            neighbors[j].val = j;
            neighbors[j].key = euclidean_distance(&query_point, &P[j], D);

            // Ordena os vizinhos pela distância
            decreaseMax((pair_t *)neighbors, j, neighbors[j]); //
        }

        // qsort(neighbors, n, sizeof(Neighbor), compare_neighbors);
        //--------

        // Armazena os índices dos k vizinhos mais próximos
        for (int m = 0; m < k; m++)
        {
            result_indices[i * k + m] = neighbors[m].val;
        }
    }

    free(neighbors);
}