#include "queue.h"

/**
 * Creates a new QueueChunk
 * RAISES: Exits if could not allocate memory for chunk
*/
struct QueueChunk* QUEUE_create_chunk(){
    struct QueueChunk* qc = malloc(sizeof(struct QueueChunk));
    if (qc == NULL){
        fprintf(stderr, "ERROR! Could not allocate enough memory for QueueChunk\n");
        exit(1);
    }
    qc->tasks = malloc(_QUEUE_CHUNK_SIZE * sizeof(void*));
    if (qc->tasks == NULL){
        fprintf(stderr, "ERROR! Could not allocate enough memory for QueueChunk\n");
        exit(1);
    }

    qc->filled = 0;
    qc->next = 0;
    qc->next_chunk = NULL;

    return qc;
}

/**
 * Creates a new Queue
 * RAISES: Exits if could not allocate memory
*/
struct Queue* QUEUE_create(){
    struct Queue* q = malloc(sizeof(struct Queue));
    if (q == NULL){
        fprintf(stderr, "ERROR! Could not allocate enough memory for Queue\n");
        exit(1);
    }

    q->front = QUEUE_create_chunk();
    q->back = q->front;
    q->dispatched = 0;
    q->live_chunk_count = 1;
    
    pthread_ready_mutex_init(&q->read_ready_mutex, NULL);
    pthread_ready_mutex_init(&q->write_mutex, NULL);
    pthread_ready_mutex_init(&q->dispatch_counter_mutex, NULL);
    pthread_ready_mutex_init(&q->live_counter_mutex, NULL);

    return q;
}

/**
 * Frees a chunk allocated by QUEUE_create_chunk
 * NOTE: This will recursively free every chunk that is a child of this
 * NOTE: Consider setting the queue_chunk's next_chunk to NULL to prevent this behaviour
 * NOTE: This will not free any ungetted tasks
*/
void QUEUE_free_chunk(struct QueueChunk* queue_chunk){
    if (queue_chunk == NULL) return;
    QUEUE_free_chunk(queue_chunk->next_chunk);
    free(queue_chunk->tasks);
    free(queue_chunk);
}

/**
 * Frees a chunk allocated by QUEUE_create
 * NOTE: This will not free the void* pointers stored in the queue
*/
void QUEUE_free(struct Queue* queue){
    QUEUE_free_chunk(queue->front);
    free(queue);
}

/**
 * Marks a task as completed (Basically just updates the count of dispatched tasks)
*/
void QUEUE_register_completion(struct Queue* queue){
    pthread_mutex_lock(&queue->dispatch_counter_mutex);
    --(queue->dispatched);
    pthread_mutex_unlock(&queue->dispatch_counter_mutex);
}

/**
 * Welcome to mutex hell
 * Blocks until the queue is non empty then pops out the first element
 * NOTE: This is thread safe :)
*/
void* QUEUE_get(struct Queue* queue){ // Consider using pcond
    pthread_ready_mutex_lock(&queue->read_ready_mutex);
    struct QueueChunk* front = queue->front;
    void* task = front->tasks[front->next++];

    if (front->next >= _QUEUE_CHUNK_SIZE){ // All values in chunk have been read
        queue->front = front->next_chunk;
        front->next_chunk = NULL; QUEUE_free_chunk(front);
        front = queue->front;
        pthread_mutex_lock(&queue->live_counter_mutex);
        --(queue->live_chunk_count);
        pthread_mutex_unlock(&queue->live_counter_mutex);
    }

    if (front->next < front->filled){ // i.e There are more values to be read
        pthread_mutex_unlock(&queue->read_ready_mutex);
    }

    pthread_mutex_lock(&queue->dispatch_counter_mutex);
    ++(queue->dispatched);
    pthread_mutex_unlock(&queue->dispatch_counter_mutex);
    return task;
}

/**
 * Welcome to mutex hell
 * Blocks until the queue is not full, then appends to the end of the queue
 * NOTE: This is thread safe :)
*/
void* QUEUE_add(struct Queue* queue, void* task){
    pthread_mutex_lock(&queue->write_mutex);
    
    pthread_mutex_unlock(&queue->write_mutex);
}
