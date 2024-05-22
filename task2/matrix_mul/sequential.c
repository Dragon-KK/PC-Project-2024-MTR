#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "matrix.h"
#include "options.h"
#include "common.h"

#define PRODUCTS_LOG_FILE "matrix_mul_seq.log"

#pragma region Business Logix
void multiply(struct Matrix* operand_a, struct Matrix* operand_b, struct Matrix* product);
#pragma endregion

int main(int argc, char* argv[]){
    srand(time(NULL));
    struct Options options; OPTIONS_set(&options, argc, argv);

    struct Matrix** operand_as = create_matrix_array(options.operations, options.matrix_order, options.matrix_order);
    struct Matrix** operand_bs = create_matrix_array(options.operations, options.matrix_order, options.matrix_order);
    struct Matrix** products   = create_matrix_array(options.operations, options.matrix_order, options.matrix_order);
    
    init_operand(operand_as, options.operations); init_operand(operand_bs, options.operations);
    
    long long int start = time_ms();
    
    for (long long int i = 0; i < options.operations; ++i){
        multiply(operand_as[i], operand_bs[i], products[i]);
    }
    long long int end = time_ms();
    
    printf("Time elapsed: %ldms\n", end - start);

    if (options.log_products){
        FILE* log_file = fopen(PRODUCTS_LOG_FILE, "w");

        for (long long int i = 0; i < options.operations; ++i){
            fprintf(log_file, "Operation %ld:\n", i);    
            MATRIX_print(operand_as[i], log_file);
            MATRIX_print(operand_bs[i], log_file);
            MATRIX_print(products[i]  , log_file);
        }
        fclose(log_file);
    }

    free_matrix_array(operand_as, options.operations);
    free_matrix_array(operand_bs, options.operations);
    free_matrix_array(products  , options.operations);
}

#pragma region Business Logix Impl
/**
 * Multiplies 2 matrices and stores the result in product matrix
 * RAISES: Exits if the matrices provided are not of correct dimensions
*/
void multiply(struct Matrix* operand_a, struct Matrix* operand_b, struct Matrix* product){
    if (operand_a->cols != operand_b->rows || operand_a->rows != product->rows || operand_b->cols != product->cols){
        fprintf(stderr, "ERROR! Invalid matrix dimension for multiplication\n");
        exit(1);
    }

    for (long long int row = 0; row < product->rows; ++row){
        for (long long int col = 0; col < product->cols; ++col){
            product->data[MATRIX_idx(row, col, product)] = 0;
            for (long long int k = 0; k < operand_a->cols; ++k){
                product->data[MATRIX_idx(row, col, product)] += operand_a->data[MATRIX_idx(row, k, operand_a)] * operand_b->data[MATRIX_idx(k, col, operand_b)];
            }
        }
    }
}
#pragma endregion
