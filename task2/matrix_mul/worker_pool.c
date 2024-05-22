#include "worker_pool.h"


void* _WP_run_helper_function(void* varg){
    struct WP_Argument* arg = varg;
    struct Queue* queue = arg->queue;
    void (*func)(void *) = arg->func;

    while (true){
        struct WP_TaskWrapper* tw = QUEUE_get(queue); // NOTE: This is blocking
        if (tw->task_type == WP_KILL){
            QUEUE_register_completion(queue);
            free(tw);
            break;
        }
        if (tw->task_type == WP_EXEC){
            func(tw->task);
            free(tw->task);
            free(tw);
            QUEUE_register_completion(queue);
            continue;
        }
        fprintf(stderr, "UNREACHABLE! Unexpected task_type %d\n", tw->task_type);
        exit(1);
    }
    return NULL;
}

/**
 * Creates the worker pool and spawns threads for each worker
 * RAISES: Exits if could not allocate memory
*/
struct WorkerPool* WP_create(void (*func)(void *), int thread_count){
    struct WorkerPool* wp = malloc(sizeof(struct WorkerPool));
    if (wp == NULL){
        fprintf(stderr, "ERROR! Could not allocate memory for WorkerPool\n");
        exit(1);
    }

    wp->arg = malloc(sizeof(struct WP_Argument));
    if (wp->arg == NULL){
        fprintf(stderr, "ERROR! Could not allocate memory for WorkerPool\n");
        exit(1);
    }

    wp->thread_count = thread_count;
    wp->threads = malloc(wp->thread_count * sizeof(pthread_t));
    if (wp->threads == NULL){
        fprintf(stderr, "ERROR! Could not allocate memory for WorkerPool\n");
        exit(1);
    }

    wp->arg->queue = QUEUE_create();
    wp->arg->func = func;


    for (int i = 0; i < wp->thread_count; ++i){
        pthread_create(&(wp->threads[i]), NULL, _WP_run_helper_function, (void*)wp->arg);
    }

    return wp;
}

/**
 * Adds a task to the queue
 * NOTE: The task must be a pointer to some malloced memory (since free will be called on it)
 * NOTE: If the queue is already at its maximum capacity, waits until space is made then adds the task
 * RAISES: Exits if could not allocate memory for wrapper
*/
void WP_enqueue_task(struct WorkerPool* worker_pool, void* task){
    struct WP_TaskWrapper* tw = malloc(sizeof(struct WP_TaskWrapper));
    if (tw == NULL){
        fprintf(stderr, "ERROR! Could not allocate memory for task wrapper\n");
        exit(1);
    }
    tw->task = task;
    tw->task_type = WP_EXEC;

    QUEUE_add(worker_pool->arg->queue, tw);
}

/**
 * Kills all threads after all current tasks have been finished
 * NOTE: This will not prevent you from enqueueing additional tasks (But it is useless to do so)
*/
void WP_request_stop(struct WorkerPool* worker_pool){
    for (int i = 0; i < worker_pool->thread_count; ++i){
        struct WP_TaskWrapper* tw = malloc(sizeof(struct WP_TaskWrapper));
        if (tw == NULL){
            fprintf(stderr, "ERROR! Could not allocate memory for task wrapper\n");
            exit(1);
        }

        tw->task = NULL;
        tw->task_type = WP_KILL;

        QUEUE_add(worker_pool->arg->queue, tw);
    }
}

/**
 * Joins all threads spawned
 * NOTE: You probably want to WP_request_stop before calling this
 * RAISES: You'll get random segfaults if the threads weren't created properly :)
*/
void WP_join(struct WorkerPool* worker_pool){
    for (int i = 0; i < worker_pool->thread_count; ++i){
        pthread_join(worker_pool->threads[i], NULL);
    }
}

/**
 * Frees all memory associated with the worker pool
*/
void WP_free(struct WorkerPool* worker_pool){
    QUEUE_free(worker_pool->arg->queue);
    free(worker_pool->arg);
    free(worker_pool->threads);
    free(worker_pool);
}
