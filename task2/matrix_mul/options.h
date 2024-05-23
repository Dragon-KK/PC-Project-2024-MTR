/**
 * EE23B135 Kaushik G Iyer
 * 23/05/2024
 * 
 * Provides definitions for parsing options set by command line arguments
 * 
*/ 

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct Options{
    long long int matrix_order; // The order of the square matrix that is multiplied, it is a required argument 
    long long int operations; // The number of multiplications to do, set to 1 if the second argument is not provided
    bool log_products; // Set to true if the user inputs a number != 0 as the third argument
};
void OPTIONS_set(struct Options* options, int argc, char* argv[]);

#include "options.c"