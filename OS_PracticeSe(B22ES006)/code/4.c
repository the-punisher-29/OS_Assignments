#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <ctype.h>
#include <string.h>

#define BUFFER_SIZE 1024

// Function to convert text to uppercase
void convert_to_uppercase(char *str) {
    while (*str) {
        *str = toupper(*str);
        str++;
    }
}

// Function to reverse text
void reverse_text(char *str) {
    int length = strlen(str);
    for (int i = 0; i < length / 2; i++) {
        char temp = str[i];
        str[i] = str[length - 1 - i];
        str[length - 1 - i] = temp;
    }
}

int main() {
    int pipe1[2][2];  // Bidirectional pipe for parent-C1
    int pipe2[2][2];  // Bidirectional pipe for parent-C2
    char buffer[BUFFER_SIZE];

    // Create pipes
    if (pipe(pipe1[0]) == -1 || pipe(pipe1[1]) == -1 ||
        pipe(pipe2[0]) == -1 || pipe(pipe2[1]) == -1) {
        perror("Pipe creation failed");
        exit(1);
    }

    // Create first child process (C1)
    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("Fork failed");
        exit(1);
    }

    if (pid1 == 0) {  // Child process C1
        close(pipe1[0][1]);  // Close write end of parent-to-C1
        close(pipe1[1][0]);  // Close read end of C1-to-parent
        close(pipe2[0][0]); close(pipe2[0][1]); // Close pipe2 completely
        close(pipe2[1][0]); close(pipe2[1][1]);

        while (1) {
            ssize_t bytes_read = read(pipe1[0][0], buffer, BUFFER_SIZE);
            if (bytes_read <= 0) break;

            convert_to_uppercase(buffer);
            write(pipe1[1][1], buffer, bytes_read);
        }

        close(pipe1[0][0]);
        close(pipe1[1][1]);
        exit(0);
    }

    // Create second child process (C2)
    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("Fork failed");
        exit(1);
    }

    if (pid2 == 0) {  // Child process C2
        close(pipe2[0][1]);  // Close write end of parent-to-C2
        close(pipe2[1][0]);  // Close read end of C2-to-parent
        close(pipe1[0][0]); close(pipe1[0][1]); // Close pipe1 completely
        close(pipe1[1][0]); close(pipe1[1][1]);

        while (1) {
            ssize_t bytes_read = read(pipe2[0][0], buffer, BUFFER_SIZE);
            if (bytes_read <= 0) break;

            buffer[bytes_read] = '\0';
            reverse_text(buffer);
            write(pipe2[1][1], buffer, bytes_read);
        }

        close(pipe2[0][0]);
        close(pipe2[1][1]);
        exit(0);
    }

    // Parent process
    close(pipe1[0][0]); close(pipe1[1][1]); // Close reading end of parent-to-C1 and writing end of C1-to-parent
    close(pipe2[0][0]); close(pipe2[1][1]); // Close reading end of parent-to-C2 and writing end of C2-to-parent

    // Open input file
    int input_fd = open("input.txt", O_RDONLY);
    if (input_fd == -1) {
        perror("Failed to open input file");
        exit(1);
    }

    // Open output file
    int output_fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd == -1) {
        perror("Failed to open output file");
        exit(1);
    }

    // Read from input file and process through both children
    ssize_t bytes_read;
    while ((bytes_read = read(input_fd, buffer, BUFFER_SIZE)) > 0) {
        // Send to C1
        write(pipe1[0][1], buffer, bytes_read);
        
        // Receive from C1
        bytes_read = read(pipe1[1][0], buffer, BUFFER_SIZE);
        
        // Send to C2
        write(pipe2[0][1], buffer, bytes_read);
        
        // Receive from C2
        bytes_read = read(pipe2[1][0], buffer, BUFFER_SIZE);
        
        // Write to output file
        write(output_fd, buffer, bytes_read);
    }

    // Close all remaining pipe ends
    close(pipe1[0][1]); close(pipe1[1][0]);
    close(pipe2[0][1]); close(pipe2[1][0]);
    close(input_fd);
    close(output_fd);

    return 0;
}