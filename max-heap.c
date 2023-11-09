#include <stdio.h>
#include <math.h>

#include "max-heap.h"
// (c) by W.Zola set/23
//
// EXAMPLE Sequential MAX-Heap, 
//                    decreaseMax operation 
//                    and others

// para compilar:
// gcc -O3 max-heap.c -o max-heap -lm



void drawHeapTree( pair_t heap[], int size, int nLevels )   // FIX ME!
{
     int offset = 0;
     int space = (int) pow( 2, nLevels-1 );
     //int space = 0;
     
     int nElements = 1;
     for( int level=0; level<nLevels; level++ ) {
        
        // print all elements in this level     
        for( int i=offset; i<size && i<(offset+nElements); i++ ) {
           
           printf( "[%3d, %f]", heap[i].val, heap[i].key );
           
        }   
        printf( "\n" );
        
        offset += nElements;
        space = nElements-1;
        nElements *= 2;
     }
}



void swap( pair_t *a, pair_t *b ) //__attribute__((always_inline));
{
    pair_t temp = *a;
    *a = *b;
    *b = temp;
}

void maxHeapify( pair_t heap[], int size, int i ) 
{
    while (1) {
        int largest = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        if (left < size && heap[left].key > heap[largest].key )
            largest = left;

        if (right < size && heap[right].key > heap[largest].key )
            largest = right;

        if (largest != i) {
            swap(&heap[i], &heap[largest]); // Using the swap function
            i = largest;
            
        } else {
            break;
        }
    }
}

//#define parent(pos) ( pos/2 ) // SE nao usar posicao 0
#define parent(pos) ( (pos-1)/2 )
        
void heapifyUp( pair_t heap[], int *size, int pos ) 
{
    pair_t val = heap[pos];
    
    while(pos>0 && val.key > heap[ parent(pos)].key){
        heap[pos] = heap[ parent(pos) ];
        pos = parent(pos);
    }
    heap[pos] = val;
}
        
void insert( pair_t heap[], int *size, pair_t element ) 
{
    *size += 1;
    int last = *size - 1;
    
    heap[last] = element; 
    heapifyUp( heap, size, last );
}
        

int isMaxHeap( pair_t heap[], int size ) 
{   
    for( int i=1; i<size; i++ )
        if( heap[i].key <= heap[parent(i)].key )
           continue;
        else {
           printf( "\nbroke at [%d]=%d\n", i, heap[i].val );
           printf( "father at [%d]=%d\n", 
                             parent(i), heap[parent(i)].val );
           return 0;
        }   
    return 1;       
}


void decreaseMax(pair_t heap[], int size, pair_t new_value) {
    if (size == 0) // Heap is empty
        return;

    if( heap[0].key > new_value.key ) {
      heap[0] = new_value;
      #if SHOW_DECREASE_MAX_STEPS 
         drawHeapTree( heap, size, 4 );
         printf( "    ~~~~~~~~~~~~~~~~~~~~~~~~~\n" );
      #endif
      maxHeapify(heap, size, 0);
    }  
}