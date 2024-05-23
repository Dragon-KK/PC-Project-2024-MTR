#include "options.h"

/**
 * Sets options based on command line arguments
 * RAISES: Exits on invalid arguments
*/
void OPTIONS_set(struct Options* options, int argc, char* argv[]){
    options->matrix_order = (argc > 1)? atoi(argv[1]) : 0;
    options->operations   = (argc > 2)? atoi(argv[2]) : 1;
    options->log_products = (argc > 3)? atoi(argv[3]) : false;
    
    if (options->matrix_order <= 0 || options->operations <= 0){
        fprintf(stderr, "ERROR! Invalid arguments to `%s`. Expected usage: `%s matrix_order{number > 0} operations{number > 0} log_products{number != 0?}`\n", argv[0], argv[0]);
        exit(1);
    }
}