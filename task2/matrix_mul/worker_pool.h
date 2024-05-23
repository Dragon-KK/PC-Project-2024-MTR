/**
 * EE23B135 Kaushik G Iyer
 * 23/05/2024
 * 
 * Provides definitions for WorkerPool
 * 
*/ 


#pragma once
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "queue.h"

struct WP_Argument{
    struct Queue* queue; // The queue of tasks that the WorkerPool is to do
    void (*func)(void *); // The function that operates on each task
};

enum WP_TaskType{
    WP_EXEC, // Requests the worker to finish a task
    WP_KILL, // Notifies the worker to just kill itself
};

struct WP_TaskWrapper{
    void* task; // The task to be accomplished
    enum WP_TaskType task_type; // I'm bored
};

struct WorkerPool{
    pthread_t* threads; // An array of all pthreads spawned
    int thread_count; // A count of number of threads spawned
    struct WP_Argument* arg; // Passed to each worker when their thread is spawned
};

void* _WP_run_helper_function(void* arg);
struct WorkerPool* WP_create(void (*func)(void *), int thread_count);
void WP_enqueue_task(struct WorkerPool* worker_pool, void* task);
void WP_request_stop(struct WorkerPool* worker_pool);
void WP_join(struct WorkerPool* worker_pool);
void WP_free(struct WorkerPool* worker_pool);

#include "worker_pool.c"