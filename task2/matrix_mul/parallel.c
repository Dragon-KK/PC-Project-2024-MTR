#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "matrix.h"
#include "options.h"
#include "common.h"
#include "worker_pool.h"

#define PRODUCTS_LOG_FILE "matrix_mul_par.log"

#pragma region Business Logix
void request_multiplication(struct Matrix* operand_a, struct Matrix* operand_b, struct Matrix* product);
#pragma endregion

int main(int argc, char* argv[]){
    srand(time(NULL));
    struct Options options; OPTIONS_set(&options, argc, argv);

    struct Matrix** operand_as = create_matrix_array(options.operations, options.matrix_order, options.matrix_order);
    struct Matrix** operand_bs = create_matrix_array(options.operations, options.matrix_order, options.matrix_order);
    struct Matrix** products   = create_matrix_array(options.operations, options.matrix_order, options.matrix_order);
    
    init_operand(operand_as, options.operations); init_operand(operand_bs, options.operations);
    
    struct WorkerPool* worker_pool = WP_create("function", 0);

    clock_t start = clock();
    
    for (long long int i = 0; i < options.operations; ++i){
        
        request_multiplication(operand_as[i], operand_bs[i], products[i]);
        // WP_enqueue_task((void*) ptask);
    }
    WP_request_stop(worker_pool);
    WP_join(worker_pool);
    
    clock_t end = clock();
    
    printf("Time elapsed: %ldms\n", (end - start) / (CLOCKS_PER_SEC / 1000));

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

    WP_free(worker_pool);

    free_matrix_array(operand_as, options.operations);
    free_matrix_array(operand_bs, options.operations);
    free_matrix_array(products  , options.operations);
}

#pragma region Business Logix Impl
/**
 * Enqueues the multiplicatin operation to the worker pool
 * RAISES: Exits if the matrices provided are not of correct dimensions
*/
void request_multiplication(struct Matrix* operand_a, struct Matrix* operand_b, struct Matrix* product){
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
