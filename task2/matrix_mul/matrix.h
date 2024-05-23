/**
 * EE23B135 Kaushik G Iyer
 * 23/05/2024
 * 
 * Provides definitions for creating and managing matrices
 * 
*/ 

#pragma once
#include <stdio.h>
#include <stdlib.h>

struct Matrix{
    long long int rows; // The number of rows in the matrix
    long long int cols; // The number of columns in the matrix
    long long int* data; // An array that stores the matrix data (flattened)
};

struct Matrix* MATRIX_create(long long int rows, long long int cols);
void MATRIX_free(struct Matrix* matrix);
long long int MATRIX_idx(long long int row, long long int col, struct Matrix* matrix);
void MATRIX_print(struct Matrix* matrix, FILE* fd);

#include "matrix.c"