#include "common.h"

/**
 * Gets current time in ms
 * NOTE: This is used over clock() as clock returned the sum of cpu time used in all threads (Which is not what we wanted to measure)
 * Yeeted from: https://stackoverflow.com/a/44896326
*/
long long time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}

/**
 * Generates a smallish random number lamo
*/
int random_number(){
    return (rand()%11) - 5;
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
 * Initializes an array of matrices
 * NOTE: This will not set values in the matrices
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
 * Frees all memory associated with the matrix array
*/
void free_matrix_array(struct Matrix** array, long long int size){
    for (long long int i = 0; i < size; ++i){
        MATRIX_free(array[i]);
    }
    free(array);
}