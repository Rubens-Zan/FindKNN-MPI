void *findKLeastPartialElmts(void *ptr); 
void concatenateOutputPortions(); 
pair_t * parallel_findKLeast( 
    const float *Input, 
    const pair_t *Output,   // pair_t é o tipo de um par (v,p)
    int nTotalElmts,
    int k, 
int nThreads);
