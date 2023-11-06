#include <stdio.h>
#include <pthread.h>

#include <unistd.h>
#include <sys/syscall.h>

#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>

#include "chrono.h"
#include "max-heap.h"
#include "findKLeast.h"

int main (int argc, char *argv[]) {
    int inputSize = 0;
    chronometer_t runningTime; 
    int USAR_PARALELO = 1;
    // const float *Input = []; 
    // const pair_t *Output = [];
    int nTotalElements = 10;  
    int k = 10; 
    int nThreads = 1;
    
    // parallel_findKLeast(Input, Output, nTotalElements, k, nThreads); 

    return 1; 
}