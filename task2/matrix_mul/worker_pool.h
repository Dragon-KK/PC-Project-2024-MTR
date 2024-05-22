/**
 * Contains definitions that are used for the creation and management of worker pools
*/

#pragma once
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "queue.h"

struct WP_Argument{
    struct Queue* queue;
    void (*func)(void *);
};

enum WP_TaskType{
    WP_EXEC,
    WP_KILL
};

struct WP_TaskWrapper{
    void* task;
    enum WP_TaskType task_type;
};

struct WorkerPool{
    pthread_t* threads;
    int thread_count;
    struct WP_Argument* arg;
};

void* _WP_run_helper_function(void* arg);
struct WorkerPool* WP_create(void (*func)(void *), int thread_count);
void WP_enqueue_task(struct WorkerPool* worker_pool, void* task);
void WP_request_stop(struct WorkerPool* worker_pool);
void WP_join(struct WorkerPool* worker_pool);
void WP_free(struct WorkerPool* worker_pool);

#include "worker_pool.c"