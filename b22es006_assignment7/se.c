#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHM_SIZE 1024  // Shared memory size

// Function to add a timestamp to the shared memory data
void add_timestamp(char *str) {
    time_t now;
    time(&now);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S]", localtime(&now));
    //strcat(str, " ");
    //strcat(str, timestamp);
}

int main() {
    key_t key = ftok("shmfile", 65);  // Generate a unique key for shared memory
    int shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);  // Create shared memory segment

    if (shmid == -1) {
        perror("Shared memory creation failed");
        exit(1);
    }

    // Attach to the shared memory
    char *str = (char*) shmat(shmid, (void*)0, 0);
    if (str == (char*)-1) {
        perror("Shared memory attach failed");
        exit(1);
    }

    // Input string from the user
    printf("Enter a string: ");
    fgets(str, SHM_SIZE, stdin);
    str[strcspn(str, "\n")] = 0;  // Remove newline character if present

    // Add timestamp to the string
    add_timestamp(str);

    // Display confirmation
    printf("Data written to shared memory: %s\n", str);

    // Detach from shared memory
    if (shmdt(str) == -1) {
        perror("Shared memory detach failed");
        exit(1);
    }

    return 0;
}
