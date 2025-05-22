#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t p1, p2;
    int status, choice;

    printf("P0 (Parent) created. PID: %d\n", getpid());

    p1 = fork();
    if (p1 == 0) {
        printf("P1 (Child) created. PID: %d\n PPID: %d\n", getpid(),getppid());
        sleep(10);  // Sleep to keep the child running
        exit(1);
    }

    p2 = fork();
    if (p2 == 0) {
        printf("P2 (Child) created. PID: %d\n PPID: %d\n", getpid(),getppid());
        sleep(10);  // Sleep to keep the child running
        exit(2);
    }

    printf("Enter 1 to wait for P1 or 2 to wait for P2: \n");
    scanf("%d", &choice);

    if (choice == 1) {
        waitpid(p1, &status, 0);
        printf("P1 (Child) terminated\n");
    } else if (choice == 2) {
        waitpid(p2, &status, 0);
        printf("P2 (Child) terminated\n");
    } else {
        printf("Invalid choice. P0 will wait for any child.\n");
        wait(&status);
    }

    printf("P0 (Parent) terminated\n");
    return 0;
}