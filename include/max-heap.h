#ifndef __MAX_HEAP__
#define __MAX_HEAP__

#include <math.h>

#define SHOW_DECREASE_MAX_STEPS 0
#define MAX_HEAP_SIZE (1024*1024)

#define FLOAT 1
#define TYPE FLOAT 

typedef struct {
    float key;   // inserir um valor v float na chave 
                 //  (obtido do vetor de entrada Input)
    int val;     // inserir a posiçao p como valor val
} pair_t;



void drawHeapTree( pair_t heap[], int size, int nLevels ) ;  // FIX ME!
void swap( pair_t *a, pair_t *b ); //__attribute__((always_inline));
void maxHeapify( pair_t heap[], int size, int i ) ;
void heapifyUp( pair_t heap[], int *size, int pos ) ;
void insert( pair_t heap[], int *size, pair_t element ) ;
void decreaseMax(pair_t heap[], int size, pair_t new_value) ;
int isMaxHeap( pair_t heap[], int size ) ;

#endif