#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "matrix.h"

/**
 * Generates a smallish random number lamo
*/
int random_number(){
    return (int)(((double)rand() / (double)RAND_MAX) * (int)((double)rand() / ((double)RAND_MAX + 1) * 10)) - 3;
}

/**
 * Sets random values for the operand matrix array
*/
void init_operand(struct Matrix** operand_array, long long int size){
    for (long long int i = 0; i < size; ++i){
        for (long long int idx = 0; idx < operand_array[i]->cols * operand_array[i]->rows; ++idx){
            operand_array[i]->data[idx] = random_number();
        }
    }
}


/**
 * Allocates memory for all the list of matrices
 * NOTE: This will not initial the values set in the matrices
 * RAISES: Exits if could not allocate memory
*/
struct Matrix** create_matrix_array(long long int size, long long int rows, long long int cols){
    struct Matrix** array = malloc(size * sizeof(struct Matrix*));

    if (array == NULL){
        fprintf(stderr, "ERROR! Could not allocated memory for the matrices :(\n");
        exit(1);
    }
    
    for (long long int i = 0; i < size; ++i){
        array[i] = MATRIX_create(rows, cols);
    }
    return array;
}
/**
 * Frees any memory associated with the matrix array
*/
void free_matrix_array(struct Matrix** array, long long int size){
    for (long long int i = 0; i < size; ++i){
        MATRIX_free(array[i]);
    }
    free(array);
}