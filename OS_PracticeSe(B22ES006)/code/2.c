#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>

#define ARRAY_SIZE 224
#define THREAD_COUNT 4
#define SUB_MATRIX_SIZE 3

// Structure to pass data to threads
typedef struct {
    int start_idx;
    int end_idx;
    int dimension;
    double *array3D;
    double *result;
} ThreadData;

// Global 3D array and result matrices
double array3D[ARRAY_SIZE][ARRAY_SIZE][ARRAY_SIZE];
double R1[ARRAY_SIZE-2][ARRAY_SIZE-2];
double R2[ARRAY_SIZE-2][ARRAY_SIZE-2];
double R3[ARRAY_SIZE-2][ARRAY_SIZE-2];
double final_result[ARRAY_SIZE-2][ARRAY_SIZE-2];

// Function to compute dot product of 3x3 submatrix
void* compute_dot_product(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    
    for(int i = data->start_idx; i < data->end_idx; i++) {
        for(int j = 1; j < ARRAY_SIZE-1; j++) {
            for(int k = 1; k < ARRAY_SIZE-1; k++) {
                double sum = 0;
                for(int di = -1; di <= 1; di++) {
                    for(int dj = -1; dj <= 1; dj++) {
                        if(data->dimension == 1)
                            sum += array3D[i+di][j][k] * array3D[i+di][j][k];
                        else if(data->dimension == 2)
                            sum += array3D[i][j+dj][k] * array3D[i][j+dj][k];
                        else
                            sum += array3D[i][j][k+dj] * array3D[i][j][k+dj];
                    }
                }
                data->result[(i-1)*(ARRAY_SIZE-2) + (j-1)] = sum;
            }
        }
    }
    return NULL;
}

// Function to process one dimension
void process_dimension(int dimension, double* result) {
    pthread_t threads[THREAD_COUNT];
    ThreadData thread_data[THREAD_COUNT];
    
    int chunk_size = (ARRAY_SIZE-2) / THREAD_COUNT;
    
    // Create threads
    for(int i = 0; i < THREAD_COUNT; i++) {
        thread_data[i].start_idx = 1 + i * chunk_size;
        thread_data[i].end_idx = (i == THREAD_COUNT-1) ? ARRAY_SIZE-1 : 1 + (i+1) * chunk_size;
        thread_data[i].dimension = dimension;
        thread_data[i].array3D = &array3D[0][0][0];
        thread_data[i].result = result;
        
        pthread_create(&threads[i], NULL, compute_dot_product, &thread_data[i]);
    }
    
    // Join threads
    for(int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
}

int main() {
    // Initialize 3D array with some values
    for(int i = 0; i < ARRAY_SIZE; i++)
        for(int j = 0; j < ARRAY_SIZE; j++)
            for(int k = 0; k < ARRAY_SIZE; k++)
                array3D[i][j][k] = (i + j + k) % 10;
    
    pid_t pid1, pid2, pid3;
    
    // Create shared memory segments
    int shmid1 = shmget(IPC_PRIVATE, sizeof(R1), IPC_CREAT | 0666);
    int shmid2 = shmget(IPC_PRIVATE, sizeof(R2), IPC_CREAT | 0666);
    int shmid3 = shmget(IPC_PRIVATE, sizeof(R3), IPC_CREAT | 0666);
    
    double *shared_R1 = (double*)shmat(shmid1, NULL, 0);
    double *shared_R2 = (double*)shmat(shmid2, NULL, 0);
    double *shared_R3 = (double*)shmat(shmid3, NULL, 0);
    
    // Create child processes
    pid1 = fork();
    if(pid1 == 0) {
        process_dimension(1, shared_R1);
        exit(0);
    }
    
    pid2 = fork();
    if(pid2 == 0) {
        process_dimension(2, shared_R2);
        exit(0);
    }
    
    pid3 = fork();
    if(pid3 == 0) {
        process_dimension(3, shared_R3);
        exit(0);
    }
    
    // Parent process waits for all children
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);
    
    // Compute final result
    for(int i = 0; i < ARRAY_SIZE-2; i++) {
        for(int j = 0; j < ARRAY_SIZE-2; j++) {
            final_result[i][j] = shared_R1[i*(ARRAY_SIZE-2) + j] +
                                shared_R2[i*(ARRAY_SIZE-2) + j] +
                                shared_R3[i*(ARRAY_SIZE-2) + j];
        }
    }
    
    // Print a small portion of the final result as example
    printf("Sample of final result matrix (top-left 3x3):\n");
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            printf("%8.2f ", final_result[i][j]);
        }
        printf("\n");
    }
    
    // Clean up shared memory
    shmdt(shared_R1);
    shmdt(shared_R2);
    shmdt(shared_R3);
    shmctl(shmid1, IPC_RMID, NULL);
    shmctl(shmid2, IPC_RMID, NULL);
    shmctl(shmid3, IPC_RMID, NULL);
    
    return 0;
}