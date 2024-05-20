#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

/**
 * Allocates a matrix
 * NOTE: The created matrix must be freed using `MATRIX_free`
 * RAISES: Exits if could not allocate memory
*/
struct Matrix* MATRIX_create(long long int rows, long long int cols){
    struct Matrix* matrix = malloc(sizeof(struct Matrix));
    if (matrix == NULL){
        fprintf(stderr, "ERROR! Could not allocated memory for matrix :(\n");
        exit(1);
    }
    matrix->data = malloc(rows * cols * sizeof(long long int));
    if (matrix->data == NULL){
        fprintf(stderr, "ERROR! Could not allocated memory for matrix :(\n");
        exit(1);
    }
    
    matrix->cols = cols;
    matrix->rows = rows;
}

/**
 * Frees the data allocated for the matrix
*/
void MATRIX_free(struct Matrix* matrix){
    free(matrix->data);
    free(matrix);
}

/**
 * Prints the matrix to the given fd
*/
void MATRIX_print(struct Matrix* matrix, FILE* fd){
    fprintf(fd, "Matrix<%ld, %ld>\n", matrix->rows, matrix->cols);
    for (long long int row = 0; row < matrix->rows; ++row){
        for (long long int col = 0; col < matrix->cols; ++col){
            fprintf(fd, "%ld ", matrix->data[MATRIX_idx(row, col, matrix)]);
        }
        fprintf(fd, "\n");
    }
}

/**
 * Gets the index of the element based on row and column number
*/
inline long long int MATRIX_idx(long long int row, long long int col, struct Matrix* matrix){
    return row * matrix->rows + col;
}