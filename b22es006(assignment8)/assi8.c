#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#define SHM_KEY 0x1234  // Unique key for shared memory

// Struct to hold the numbers and result for each thread
typedef struct {
    int num1;
    int num2;
    int result;
} ThreadData;

// Semaphore to protect shared data
sem_t mutex;

// Thread function to compute the sum
void* compute_sum(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    // Protect shared data with semaphore
    sem_wait(&mutex);  
    data->result = data->num1 + data->num2;  
    sem_post(&mutex);  // Release semaphore
    pthread_exit(NULL);  // Terminate the thread
}

int main() {
    pthread_t thread1, thread2;  // Thread identifiers
    ThreadData data1, data2;  // Local data for user input
    int shmid;  // Shared memory ID
    ThreadData* shared_data;  // Pointer to shared memory
    // Initialize semaphore (0 = shared between threads, 1 = initial value)
    if (sem_init(&mutex, 0, 1) != 0) {
        perror("Failed to initialize semaphore");
        exit(EXIT_FAILURE);
    }
    // Creating shared memory segment for 2 ThreadData structs
    shmid = shmget(SHM_KEY, 2 * sizeof(ThreadData), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("Failed to create shared memory");
        exit(EXIT_FAILURE);
    }
    // Attaching shared memory segment to our process
    shared_data = (ThreadData*)shmat(shmid, NULL, 0);
    if (shared_data == (ThreadData*)-1) {
        perror("Failed to attach shared memory");
        exit(EXIT_FAILURE);
    }
    //input the nos 1st pair
    printf("Enter first pair of numbers (num1 num2): ");
    if (scanf("%d %d", &data1.num1, &data1.num2) != 2) {
        fprintf(stderr, "Invalid input for first pair\n");
        exit(EXIT_FAILURE);
    }
    //input the nos 2nd pair
    printf("Enter second pair of numbers (num1 num2): ");
    if (scanf("%d %d", &data2.num1, &data2.num2) != 2) {
        fprintf(stderr, "Invalid input for second pair\n");
        exit(EXIT_FAILURE);
    }
    // Copying the input data into shared memory
    shared_data[0] = data1;
    shared_data[1] = data2;
    // Creating two threads to compute the sums
    if (pthread_create(&thread1, NULL, compute_sum, (void*)&shared_data[0]) != 0) {
        perror("Failed to create thread 1");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&thread2, NULL, compute_sum, (void*)&shared_data[1]) != 0) {
        perror("Failed to create thread 2");
        exit(EXIT_FAILURE);
    }
    // Waiting for both threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    // Retrieve the results from shared memory
    int sum1 = shared_data[0].result;
    int sum2 = shared_data[1].result;
    int total_sum = sum1 + sum2;
    // Output the results
    printf("Sum from thread 1: %d\n", sum1);
    printf("Sum from thread 2: %d\n", sum2);
    printf("Total sum of both pairs: %d\n", total_sum);
    // Detach and destroy shared memory
    if (shmdt(shared_data) == -1) {
        perror("Failed to detach shared memory");
        exit(EXIT_FAILURE);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Failed to destroy shared memory");
        exit(EXIT_FAILURE);
    }
    // Destroying the semaphore
    sem_destroy(&mutex);
    return 0;
}
