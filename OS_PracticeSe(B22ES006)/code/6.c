#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 10
#define ITERATIONS 1000

// Global variables
volatile long long counter = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Structure to hold thread-specific data
typedef struct {
    int thread_id;
    long long local_sum;
} ThreadData;

// Function for incrementing counter without lock
void* increment_without_lock(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    
    for (int i = 0; i < ITERATIONS; i++) {
        // Read current value
        long long temp = counter;
        
        // Simulate some work
        usleep(1);  // Small delay to increase chance of race condition
        
        // Increment and write back
        counter = temp + 1;
        
        data->local_sum++;
    }
    return NULL;
}

// Function for incrementing counter with lock
void* increment_with_lock(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    
    for (int i = 0; i < ITERATIONS; i++) {
        pthread_mutex_lock(&mutex);
        
        // Read current value
        long long temp = counter;
        
        // Simulate some work
        usleep(1);  // Same delay as above for fair comparison
        
        // Increment and write back
        counter = temp + 1;
        
        data->local_sum++;
        
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    
    // First run: Without locks
    counter = 0;
    printf("Running without locks...\n");
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].local_sum = 0;
        pthread_create(&threads[i], NULL, increment_without_lock, &thread_data[i]);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Calculate total increments attempted
    long long total_increments = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        total_increments += thread_data[i].local_sum;
    }
    
    printf("Expected final value: %lld\n", total_increments);
    printf("Actual final value without locks: %lld\n", counter);
    printf("Lost updates: %lld\n\n", total_increments - counter);
    
    // Second run: With locks
    counter = 0;
    printf("Running with locks...\n");
    
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].local_sum = 0;
        pthread_create(&threads[i], NULL, increment_with_lock, &thread_data[i]);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Calculate total increments attempted
    total_increments = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        total_increments += thread_data[i].local_sum;
    }
    
    printf("Expected final value: %lld\n", total_increments);
    printf("Actual final value with locks: %lld\n", counter);
    printf("Lost updates: %lld\n", total_increments - counter);
    
    pthread_mutex_destroy(&mutex);
    return 0;
}