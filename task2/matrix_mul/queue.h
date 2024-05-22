/**
 * Contains definitions for a general purpose queue that is thread safe
*/

#pragma once
#include <pthread.h>

// The max number of tasks stored in each QueueChunk
#define _QUEUE_CHUNK_SIZE 1000
// The max number of QueueChunks that can exist at a given point
#define _QUEUE_MAX_LIVE_CHUNKS 100

struct QueueChunk{
    void** tasks;
    struct QueueChunk* next_chunk;
    int filled;
    int next;
};

struct Queue{
    struct QueueChunk* front;
    struct QueueChunk* back;
    int live_chunk_count;
    int dispatched;
    pthread_mutex_t read_ready_mutex;
    pthread_mutex_t write_mutex;
    pthread_mutex_t dispatch_counter_mutex;
    pthread_mutex_t live_counter_mutex;
};

struct QueueChunk* QUEUE_create_chunk();
struct Queue* QUEUE_create();
void QUEUE_free_chunk(struct QueueChunk* queue_chunk);
void QUEUE_free(struct Queue* queue);
void* QUEUE_get(struct Queue* queue);
void* QUEUE_add(struct Queue* queue, void* task);
void QUEUE_register_completion(struct Queue* queue);

#include "queue.c"