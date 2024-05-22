#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "matrix.h"
#include "options.h"
#include "common.h"
#include "worker_pool.h"

#define PRODUCTS_LOG_FILE "matrix_mul_par.log"

// Too small and the threads wait a lot more causing bad performance and too big causes it to become basically sequential (balance is key)
#define TASK_CHUNK_SIZE 1000
// I've got 6 coures so my best performance would be with 12 threads (Anything more really doesn't improve performance) but having just a few more threads has negligible downsides
#define WORKER_POOL_THREAD_COUNT 16

#pragma region Business Logix
struct MultiplicationTask{
    struct Matrix* op1; // The premultiplicand
    struct Matrix* op2; // The postmultiplicand
    struct Matrix* res; // The matrix in which the product is to be stored
    long long int start_idx; // The start of the chunk that this task is supposed to compute
    long long int end_idx; // The end of the chunk that this task is supposed to compute
};

void sub_multiplication_handler(void* task);
void request_multiplication(struct Matrix* operand_a, struct Matrix* operand_b, struct Matrix* product, struct WorkerPool* worker_pool);
#pragma endregion

int main(int argc, char* argv[]){
    srand(time(NULL));
    struct Options options; OPTIONS_set(&options, argc, argv);

    struct Matrix** operand_as = create_matrix_array(options.operations, options.matrix_order, options.matrix_order);
    struct Matrix** operand_bs = create_matrix_array(options.operations, options.matrix_order, options.matrix_order);
    struct Matrix** products   = create_matrix_array(options.operations, options.matrix_order, options.matrix_order);
    
    init_operand(operand_as, options.operations); init_operand(operand_bs, options.operations);
    
    struct WorkerPool* worker_pool = WP_create(sub_multiplication_handler, WORKER_POOL_THREAD_COUNT);

    long long int start = time_ms();
    
    for (long long int i = 0; i < options.operations; ++i){
        request_multiplication(operand_as[i], operand_bs[i], products[i], worker_pool);
    }
    WP_request_stop(worker_pool);
    WP_join(worker_pool);
    
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

    if (worker_pool->arg->queue->dispatched != 0){
        fprintf(stderr, "ERROR! Core logic issue, there are still %d dispatched tasks\n", worker_pool->arg->queue->dispatched);
        exit(1);
    }
    if (worker_pool->arg->queue->live_chunk_count != 1){
        fprintf(stderr, "ERROR! Core logic issue, there are still %d live chunks\n", worker_pool->arg->queue->live_chunk_count);
        exit(1);
    }
    if (worker_pool->arg->queue->back->filled != worker_pool->arg->queue->back->next){
        fprintf(stderr, "ERROR! Core logic issue, there are still %d undispatched tasks\n", worker_pool->arg->queue->back->filled - worker_pool->arg->queue->back->next);
        exit(1);
    }

    WP_free(worker_pool);

    free_matrix_array(operand_as, options.operations);
    free_matrix_array(operand_bs, options.operations);
    free_matrix_array(products  , options.operations);
}

#pragma region Business Logix Impl

/**
 * Handles part of the matrix multiplication (To be run in parallel)
 * This basically calculates the result of a product
*/
void sub_multiplication_handler(void* vtask){
    struct MultiplicationTask* task = vtask;

    long long int row = task->start_idx / task->res->cols;
    long long int col = task->start_idx % task->res->cols;
    for (long long int idx = task->start_idx; idx < task->end_idx; ++idx){
        task->res->data[idx] = 0;
        for (long long int k = 0; k < task->op1->cols; ++k){
            // Just get the row and column then you can easily reuse this shit
            task->res->data[idx] += task->op1->data[MATRIX_idx(row, k, task->op1)] * task->op2->data[MATRIX_idx(k, col, task->op2)];
        }
        ++col;
        if (col == task->res->cols){
            col = 0;
            ++row;
        }
    }
}

/**
 * Enqueues the multiplication operation to the worker pool
 * RAISES: Exits if the matrices provided are not of correct dimensions or if could not allocate memory describe the task
*/
void request_multiplication(struct Matrix* operand_a, struct Matrix* operand_b, struct Matrix* product, struct WorkerPool* worker_pool){
    if (operand_a->cols != operand_b->rows || operand_a->rows != product->rows || operand_b->cols != product->cols){
        fprintf(stderr, "ERROR! Invalid matrix dimension for multiplication\n");
        exit(1);
    }

    long long int max_idx =  product->cols * product->rows;
    for (long long int idx = 0; idx < max_idx; idx += TASK_CHUNK_SIZE){
        struct MultiplicationTask* task = malloc(sizeof(struct MultiplicationTask));
        if (task == NULL){
            fprintf(stderr, "ERROR! Could not allocate memory for queueing task\n");
            exit(1);   
        }
        task->op1 = operand_a;
        task->op2 = operand_b;
        task->res = product;
        task->start_idx = idx;
        task->end_idx = (max_idx - idx >= TASK_CHUNK_SIZE)? (idx + TASK_CHUNK_SIZE) : max_idx;
        
        WP_enqueue_task(worker_pool, (void*) task);
    }
    
    
}
#pragma endregion
