/**
 * Contains utility functions that are used in both sequential and parallel
*/

#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include "matrix.h"

long long int time_ms();
int random_number();
struct Matrix** create_matrix_array(long long int size, long long int rows, long long int cols);
void free_matrix_array(struct Matrix** array, long long int size);
void init_operand(struct Matrix** operand_array, long long int size);

#include "common.c"