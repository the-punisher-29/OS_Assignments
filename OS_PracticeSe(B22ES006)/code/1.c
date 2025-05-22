#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 4

typedef struct {
    int *array;
    int start;
    int end;
    long long sum;
} ThreadData;

void *sum_array_segment(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    data->sum = 0;
    for (int i = data->start; i < data->end; i++) {
        data->sum += data->array[i];
    }
    pthread_exit(NULL);
}

int main() {
    int array_size = 1000;
    int *array = (int *)malloc(array_size * sizeof(int));
    for (int i = 0; i < array_size; i++) {
        array[i] = i + 1; // Initialize array with values 1 to 1000
    }

    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    int segment_size = array_size / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].array = array;
        thread_data[i].start = i * segment_size;
        thread_data[i].end = (i == NUM_THREADS - 1) ? array_size : (i + 1) * segment_size;
        pthread_create(&threads[i], NULL, sum_array_segment, (void *)&thread_data[i]);
    }

    long long total_sum = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        total_sum += thread_data[i].sum;
    }

    printf("Total sum: %lld\n", total_sum);

    free(array);
    return 0;
}