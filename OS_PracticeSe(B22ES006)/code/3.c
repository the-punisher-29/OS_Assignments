#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>

#define BUFFER_SIZE 1024

int main() {
    int pipefd[2];
    pid_t pid;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // Create pipe
    if (pipe(pipefd) == -1) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }

    // Create child process
    pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {  // Parent process
        close(pipefd[0]);  // Close read end

        // Open input file
        int input_fd = open("input.txt", O_RDONLY);
        if (input_fd == -1) {
            perror("Failed to open input file");
            exit(EXIT_FAILURE);
        }

        // Read from file and write to pipe
        while ((bytes_read = read(input_fd, buffer, BUFFER_SIZE)) > 0) {
            write(pipefd[1], buffer, bytes_read);
        }

        // Close file and pipe
        close(input_fd);
        close(pipefd[1]);
        
        // Wait for child to complete
        wait(NULL);
    }
    else {  // Child process
        close(pipefd[1]);  // Close write end

        // Open output file
        int output_fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (output_fd == -1) {
            perror("Failed to open output file");
            exit(EXIT_FAILURE);
        }

        // Read from pipe and write to file
        while ((bytes_read = read(pipefd[0], buffer, BUFFER_SIZE)) > 0) {
            write(output_fd, buffer, bytes_read);
        }

        // Close file and pipe
        close(output_fd);
        close(pipefd[0]);
    }

    return 0;
}