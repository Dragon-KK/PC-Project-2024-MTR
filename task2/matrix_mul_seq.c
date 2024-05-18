#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LOG_OPERATIONS 0
#define OPERATION_LOG_FILE "matrix_mul_seq.log"

#pragma region Definitions

#pragma region Matrix
struct Matrix{
    long long int* data;
    long long int rows;
    long long int cols;
};
struct Matrix* MATRIX_create(long long int rows, long long int cols);
void MATRIX_free(struct Matrix* matrix);
long long int MATRIX_idx(long long int row, long long int col, struct Matrix* matrix);
void MATRIX_print(struct Matrix* matrix, FILE* fd);
#pragma endregion

#pragma region Options
struct Options{
    long long int matrix_order;
    long long int operations;
};
void setOptions(struct Options* options, int argc, char* argv[]);
#pragma endregion

#pragma region Business Logix
void create_matrices(struct Matrix*** operand_as, struct Matrix*** operand_bs, struct Matrix*** products, struct Options* options);
void init_operand(struct Matrix** operand_array, struct Options* options);
void multiply(struct Matrix* operand_a, struct Matrix* operand_b, struct Matrix* product);
#pragma endregion

#pragma endregion


int main(int argc, char* argv[]){
    srand(time(NULL));
    struct Options options; setOptions(&options, argc, argv);

    struct Matrix **operand_as, **operand_bs, **products;
    create_matrices(&operand_as, &operand_bs, &products, &options);
    init_operand(operand_as, &options); init_operand(operand_bs, &options);
    
    clock_t start = clock();
    for (long long int i = 0; i < options.operations; ++i){
        multiply(operand_as[i], operand_bs[i], products[i]);
    }
    clock_t end = clock();
    printf("Time elapsed: %ldms\n", (end - start) / (CLOCKS_PER_SEC / 1000));

    if (LOG_OPERATIONS){
        FILE* log_file = fopen(OPERATION_LOG_FILE, "w");

        for (long long int i = 0; i < options.operations; ++i){
            fprintf(log_file, "Operation %ld:\n", i);    
            MATRIX_print(operand_as[i], log_file);
            MATRIX_print(operand_bs[i], log_file);
            MATRIX_print(products[i], log_file);
        }
        fclose(log_file);
    }

    for (long long int i = 0; i < options.operations; ++i){
        MATRIX_free(operand_as[i]);
        MATRIX_free(operand_bs[i]);
        MATRIX_free(products[i]);
    }

    free(operand_as);
    free(operand_bs);
    free(products);
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

long long int random_number(){
    return (int)(((double)rand() / (double)RAND_MAX) * (int)((double)rand() / ((double)RAND_MAX + 1) * 10)) - 3;
}

/**
 * Sets random values for the operand matrix array
*/
void init_operand(struct Matrix** operand_array, struct Options* options){
    for (long long int i = 0; i < options->operations; ++i){
        for (long long int idx = 0; idx < operand_array[i]->cols * operand_array[i]->rows; ++idx){
            operand_array[i]->data[idx] = random_number();
        }
    }
}


/**
 * Allocates memory for all the list of matrices
 * NOTE: This will not initial the values set in the matrices
 * RAISES: Exits if could not allocate memory
*/
void create_matrices(struct Matrix*** operand_as, struct Matrix*** operand_bs, struct Matrix*** products, struct Options* options){
    *operand_as = malloc(options->operations * sizeof(struct Matrix*));
    *operand_bs = malloc(options->operations * sizeof(struct Matrix*));
    *products   = malloc(options->operations * sizeof(struct Matrix*));

    if (*operand_as == NULL || *operand_bs == NULL || *products == NULL){
        fprintf(stderr, "ERROR! Could not allocated memory for the matrices :(\n");
        exit(1);
    }

    for (long long int i = 0; i < options->operations; ++i){
        (*operand_as)[i] = MATRIX_create(options->matrix_order, options->matrix_order);
        (*operand_bs)[i] = MATRIX_create(options->matrix_order, options->matrix_order);
        (*products)[i]   = MATRIX_create(options->matrix_order, options->matrix_order);
    }
}
#pragma endregion

#pragma region Matrix Impl
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
#pragma endregion

#pragma region Options Impl
/**
 * Sets options based on command line arguments
 * RAISES: Exits on invalid arguments
*/
void setOptions(struct Options* options, int argc, char* argv[]){
    options->matrix_order = (argc > 1)? atoi(argv[1]) : 0;
    options->operations   = (argc > 2)? atoi(argv[2]) : 1;
    
    if (options->matrix_order <= 0 || options->operations <= 0){
        fprintf(stderr, "ERROR! Invalid arguments to `%s`. Expected usage: `%s matrix_order{number > 0} operations{number > 0}`\n", argv[0], argv[0]);
        exit(1);
    }
}
#pragma endregion
