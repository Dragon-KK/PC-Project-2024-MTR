/**
 * EE23B135 Kaushik G Iyer
 * 23/05/2024
 * 
 * Queue moment frfr
 * Provides definitions for a thread safe queue
 * FUTURE: Some optimizations can be made (by using more mutexes to allow for more concurrency), but this works for now :)
 * 
*/ 

#pragma once
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

/**
 * The max number of tasks stored in each QueueChunk
 * Too low and you will have a huge number of malloc calls. Too high and you will waste space
*/
#define _QUEUE_CHUNK_SIZE 1000

/**
 * The max number of QueueChunks that can exist at a given point
 * Too low and you will have your queue_addition wait a lot and too high would mean high memory usage
*/
#define _QUEUE_MAX_LIVE_CHUNKS 1000

struct QueueChunk{
    void** tasks; // Stores an array of data
    struct QueueChunk* next_chunk; // Pretty self explanatory ngl
    int filled; // The amount of elements fill in the chunk
    int next; // Index of the next element to be read
};

struct Queue{
    struct QueueChunk* front;
    struct QueueChunk* back;
    int live_chunk_count; // A count of the number of chunks that are allocated
    int dispatched; // A count of number of tasks that are currently being worked on
    pthread_cond_t read_ready_cond; // Signal that is broadcasted when data is added to the queue
    pthread_cond_t write_ready_cond; // Signal that is broadcasted when data is popped from the queue
    pthread_mutex_t rw_mutex; // read-write mutex
    pthread_mutex_t dispatch_counter_mutex; // Mutex that protects `dispatched`
    pthread_mutex_t live_counter_mutex; // Mutex that protects `live_chunk_count`
};

struct QueueChunk* QUEUE_create_chunk();
struct Queue* QUEUE_create();
void QUEUE_free_chunk(struct QueueChunk* queue_chunk);
void QUEUE_free(struct Queue* queue);
void* QUEUE_get(struct Queue* queue);
void* QUEUE_add(struct Queue* queue, void* task);
void QUEUE_register_completion(struct Queue* queue);

#include "queue.c"