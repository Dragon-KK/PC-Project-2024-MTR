/**
 * Contains definitions for handling matrices
*/

#pragma once

struct Matrix{
    long long int rows;
    long long int cols;
    long long int* data;
};

struct Matrix* MATRIX_create(long long int rows, long long int cols);
void MATRIX_free(struct Matrix* matrix);
long long int MATRIX_idx(long long int row, long long int col, struct Matrix* matrix);
void MATRIX_print(struct Matrix* matrix, FILE* fd);

#include "matrix.c"