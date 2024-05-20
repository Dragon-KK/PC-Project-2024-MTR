/**
 * Contains definitions for handling command line arguments
*/

#pragma once
#include <stdbool.h>

struct Options{
    long long int matrix_order;
    long long int operations;
    bool log_products;
};
void OPTIONS_set(struct Options* options, int argc, char* argv[]);

#include "options.c"