#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>  
int main() {
    pid_t pid1, pid2;
    // P0: Grandparent process
    printf("P0: Grandparent Process. PID: %d, PPID: %d\n", getpid(), getppid());
    // Creating first child (P1: Parent process)
    pid1 = fork();
    if (pid1 == 0) {
        // P1: Parent process (child of P0)
        printf("P1: Parent Process. PID: %d, PPID: %d\n", getpid(), getppid());
        // Creating second child (P2: Child process)
        pid2 = fork();
        if (pid2 == 0) {
            // P2: Child process (child of P1)
            printf("P2: Child Process. PID: %d, PPID: %d\n", getpid(), getppid());
        } 
        else {
            wait(NULL);
        }
    } 
    else {
        wait(NULL);
    }
    return 0;
}

