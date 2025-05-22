#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

// Constants
#define NUM_PHILOSOPHERS 5
#define SIMULATION_TIME 30  //a fixed time period time is period is defined to obtain results for test cases 3 and 5

// Global variables for all test cases
pthread_mutex_t forks[NUM_PHILOSOPHERS];  // Mutex for each fork
pthread_mutex_t lock;  // For fairness in Test Case 3
int eat_count[NUM_PHILOSOPHERS] = {0};  // Track how many times each philosopher eats
int running = 1;  // Flag for simulation time control
double total_wait_time[NUM_PHILOSOPHERS] = {0};  // toTrack wait times for Test Case 5

// Struct to hold philosopher's information
typedef struct {
    int id;
    int is_fair;
} philosopher_info;

// Utility function to get current time for wait time calculations (Test Case 5)
double get_current_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// Philosopher behavior(common for test cases)
void* philosopher_common(void* arg) {
    philosopher_info* info = (philosopher_info*)arg;
    int id = info->id;
    int is_fair = info->is_fair;

    while (running) {
        printf("Philosopher %d is thinking.\n", id + 1);
        sleep(1);  // Simulate thinking

        if (is_fair) pthread_mutex_lock(&lock);  // Fairness lock for Test Case 3

        double start_wait_time = get_current_time();  // For Test Case 5 wait time
        // Pick up forks based on even/odd IDs
        if (id % 2 == 0) {
            pthread_mutex_lock(&forks[id]);
            pthread_mutex_lock(&forks[(id + 1) % NUM_PHILOSOPHERS]);
        } else {
            pthread_mutex_lock(&forks[(id + 1) % NUM_PHILOSOPHERS]);
            pthread_mutex_lock(&forks[id]);
        }

        double end_wait_time = get_current_time();  // For Test Case 5 wait time
        total_wait_time[id] += end_wait_time - start_wait_time;

        printf("Philosopher %d is eating.\n", id + 1);
        eat_count[id]++;
        sleep(2);  // Simulate eating

        // Put down forks in reverse order
        if (id % 2 == 0) {
            pthread_mutex_unlock(&forks[(id + 1) % NUM_PHILOSOPHERS]);
            pthread_mutex_unlock(&forks[id]);
        } else {
            pthread_mutex_unlock(&forks[id]);
            pthread_mutex_unlock(&forks[(id + 1) % NUM_PHILOSOPHERS]);
        }

        if (is_fair) pthread_mutex_unlock(&lock);  // Unlock fairness lock
        printf("Philosopher %d has finished eating.\n", id + 1);
        sleep(1);  // Simulate thinking time before trying again
    }

    return NULL;
}

// Test Case 1: Basic Synchronization Test
void test_case_1() {
    printf("\n******** Running Test Case 1 ********\n");
    pthread_t philosophers[NUM_PHILOSOPHERS];
    philosopher_info philosopher_ids[NUM_PHILOSOPHERS];

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_init(&forks[i], NULL);
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        philosopher_ids[i].id = i;
        philosopher_ids[i].is_fair = 0;  // Not a fair scenario
        pthread_create(&philosophers[i], NULL, philosopher_common, &philosopher_ids[i]);
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(philosophers[i], NULL);
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_destroy(&forks[i]);
    }
}

// Test Case 2: Deadlock Avoidance Test
// Test Case 2: Deadlock Avoidance Test with Timeout after each philosopher eats once
void test_case_2() {
    printf("\n******** Running Test Case 2 ********\n");
    pthread_t philosophers[NUM_PHILOSOPHERS];
    philosopher_info philosopher_ids[NUM_PHILOSOPHERS];

    // Initialize mutexes for forks
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_init(&forks[i], NULL);
    }

    // Reset eat counts and initialize the philosophers
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        eat_count[i] = 0;
        philosopher_ids[i].id = i;
        philosopher_ids[i].is_fair = 0;  // Not a fair scenario
        pthread_create(&philosophers[i], NULL, philosopher_common, &philosopher_ids[i]);
    }

    // Timeout mechanism: checking for a timeout while allowing philosophers to eat
    int timeout_seconds = 10;  // Set a timeout period
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);  // Get the start time

    int all_eaten = 0;  // Flag to check if all philosophers have eaten at least once

    // Wait until all philosophers have eaten at least once or timeout occurs
    while (!all_eaten) {
        // Assume all philosophers have eaten
        all_eaten = 1;

        // Check each philosopher to see if they've eaten at least once
        for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
            if (eat_count[i] == 0) {
                all_eaten = 0;  // If any philosopher hasn't eaten, set flag to 0
                break;
            }
        }

        // Check the current time
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        double elapsed_time = (end_time.tv_sec - start_time.tv_sec) + 
                              (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

        // If the timeout period has been exceeded, break out of the loop
        if (elapsed_time > timeout_seconds) {
            printf("Timeout: Not all philosophers have eaten within %d seconds.\n", timeout_seconds);
            break;
        }

        // Sleep briefly to avoid busy waiting
        usleep(100000);  // Sleep for 100 milliseconds
    }

    // Stop all philosopher threads if timeout occurs or all have eaten
    running = 0;

    // Join philosopher threads
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(philosophers[i], NULL);
    }

    // Log the results
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        printf("Philosopher %d ate %d times.\n", i + 1, eat_count[i]);
    }

    // Destroy mutexes
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_destroy(&forks[i]);
    }

    // Log a message indicating whether all philosophers have eaten at least once
    if (all_eaten) {
        printf("All philosophers have eaten at least once within the timeout.\n");
    } else {
        printf("Not all philosophers have eaten at least once.\n");
    }
}


// Test Case 3: Fair Eating with Equal Chances for Philosophers
void test_case_3() {
    printf("\n******** Running Test Case 3 ********\n");
    pthread_t philosophers[NUM_PHILOSOPHERS];
    philosopher_info philosopher_ids[NUM_PHILOSOPHERS];

    // Initialize mutexes for forks
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_init(&forks[i], NULL);
    }

    // Reset eat counts before starting the test
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        eat_count[i] = 0;
    }

    // Start the philosopher threads
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        philosopher_ids[i].id = i;
        philosopher_ids[i].is_fair = 1;  // Enable fairness for this test
        pthread_create(&philosophers[i], NULL, philosopher_common, &philosopher_ids[i]);
    }

    // Run the test for a fixed period (SIMULATION_TIME)
    sleep(SIMULATION_TIME);

    // Stop all philosopher threads after the simulation time
    running = 0;
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(philosophers[i], NULL);
    }

    // Destroy mutexes
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_destroy(&forks[i]);
    }

    // Output the number of times each philosopher ate
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        printf("Philosopher %d ate %d times.\n", i + 1, eat_count[i]);
    }

    // Check for fairness
    int max_eats = eat_count[0], min_eats = eat_count[0];
    for (int i = 1; i < NUM_PHILOSOPHERS; i++) {
        if (eat_count[i] > max_eats) {
            max_eats = eat_count[i];
        }
        if (eat_count[i] < min_eats) {
            min_eats = eat_count[i];
        }
    }

    // Ensure that no philosopher starved and the eating counts are roughly equal
    if ((max_eats - min_eats) <= 1) {
        printf("Test Case 3 Passed: All philosophers ate a roughly equal number of times.\n");
    } else {
        printf("Test Case 3 Failed: There is a significant discrepancy in eating counts.\n");
    }
}

// Test Case 4: Concurrent Eating Test
void test_case_4() {
    printf("\n******** Running Test Case 4 ********\n");
    pthread_t philosophers[NUM_PHILOSOPHERS];
    philosopher_info philosopher_ids[NUM_PHILOSOPHERS];

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_init(&forks[i], NULL);
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        philosopher_ids[i].id = i;
        philosopher_ids[i].is_fair = 0;  // Not a fair scenario
        pthread_create(&philosophers[i], NULL, philosopher_common, &philosopher_ids[i]);
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(philosophers[i], NULL);
    }

    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_destroy(&forks[i]);
    }
}



// Test Case 5: Efficiency Test with Wait Time Measurement
void test_case_5() {
    printf("\n******** Running Test Case 5 ********\n");
    pthread_t philosophers[NUM_PHILOSOPHERS];
    philosopher_info philosopher_ids[NUM_PHILOSOPHERS];

    // Initialize mutexes for forks
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_init(&forks[i], NULL);
    }

    // Reset wait times and eat counts
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        total_wait_time[i] = 0;
        eat_count[i] = 0;
    }

    // Start the philosopher threads
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        philosopher_ids[i].id = i;
        philosopher_ids[i].is_fair = 0;  // No fairness enforcement for this test
        pthread_create(&philosophers[i], NULL, philosopher_common, &philosopher_ids[i]);
    }

    // Run the test for a fixed period (SIMULATION_TIME)
    sleep(SIMULATION_TIME);

    // Stop all philosopher threads after the simulation time
    running = 0;
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(philosophers[i], NULL);
    }

    // Destroy mutexes
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_destroy(&forks[i]);
    }

    // Output the number of times each philosopher ate and their total wait times
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        printf("Philosopher %d ate %d times and waited %.2f seconds.\n", i + 1, eat_count[i], total_wait_time[i]);
    }
}

int main() {
    //uncomment each test case and run them individually to get respective outputs
    //test_case_1();
    test_case_2();
    //test_case_3();
    //test_case_4();
    //test_case_5();

    return 0;
}
