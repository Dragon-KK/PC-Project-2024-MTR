#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


volatile int counter = 0;
pthread_mutex_t counter_mutex;
int num_increments = 0;

void *increment_counter(void *arg) 
{
    // Notice that the race condition might not be evident for low num_increments as by the time the second thread spawns the first thread finished execution
    for (int i = 0; i < num_increments; i++) 
    {
        pthread_mutex_lock(&counter_mutex); // We need stop both threads from setting the counter at the same time
        counter++; // Using mutexs allow us to stop race conditions
        pthread_mutex_unlock(&counter_mutex);
    }
    return NULL; // It kept on throwing an illegal instruction message when I joined this thread without explicitly returning NULL
    // Idk why it didn't implicitly return NULL
}

int main(int argc, char *argv[]) 
{
    if (argc != 2) 
    {
        fprintf(stderr, "Usage: %s <num_increments>\n", argv[0]);
        return EXIT_FAILURE;
    }

    num_increments = atoi(argv[1]);

    pthread_t thread1, thread2;
    pthread_mutex_init(&counter_mutex, NULL);

    pthread_create(&thread1, NULL, &increment_counter, &num_increments);
    pthread_create(&thread2, NULL, &increment_counter, &num_increments);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Final counter value: %d\n", counter);

    return 0;
}