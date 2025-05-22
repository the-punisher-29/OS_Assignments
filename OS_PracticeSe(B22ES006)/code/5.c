#include <stdio.h>
#include <pthread.h>

// Shared variable
int counter = 0;

// Function to increment counter
void* inc(void* arg) {
    int temp = counter;
    temp = temp + 1;
    counter = temp;
    printf("After increment: %d\n", counter);
    return NULL;
}

// Function to decrement counter
void* dec(void* arg) {
    int temp = counter;
    temp = temp - 1;
    counter = temp;
    printf("After decrement: %d\n", counter);
    return NULL;
}

int main() {
    pthread_t t1, t2;

    // Create two threads
    pthread_create(&t1, NULL, inc, NULL);
    pthread_create(&t2, NULL, dec, NULL);

    // Wait for both threads to complete
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // Print final value
    printf("Final counter value: %d\n", counter);

    return 0;
}