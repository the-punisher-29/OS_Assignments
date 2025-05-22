#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SIZE 1024  // Shared memory size

typedef struct {
    char *word;
    int original_index;
} WordInfo;

// Function to find the largest word
void find_largest_word(WordInfo words[], int word_count, char *largest_word) {
    strcpy(largest_word, words[0].word);
    for (int i = 1; i < word_count; i++) {
        if (strlen(words[i].word) > strlen(largest_word)) {
            strcpy(largest_word, words[i].word);
        }
    }
}

// Function to sort words by length, preserving original order for equal lengths
void sort_words_by_length(WordInfo words[], int word_count, int ascending) {
    for (int i = 0; i < word_count - 1; i++) {
        for (int j = i + 1; j < word_count; j++) {
            int length_i = strlen(words[i].word);
            int length_j = strlen(words[j].word);
            // Sort by length; if lengths are equal, keep original order
            if ((ascending && (length_i > length_j || (length_i == length_j && words[i].original_index > words[j].original_index))) ||
                (!ascending && (length_i < length_j || (length_i == length_j && words[i].original_index > words[j].original_index)))) {
                WordInfo temp = words[i];
                words[i] = words[j];
                words[j] = temp;
            }
        }
    }
}

// Function to log the original input string for reference
void log_input(const char *input) {
    printf("[LOG] Original input: %s\n", input);
}

int main() {
    key_t key = ftok("shmfile", 65); // Generate a unique key for shared memory
    int shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT); // Create shared memory segment
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
    printf("Data read from shared memory: %s\n", str);

    // Log the input for tracking
    //log_input(str);

    // Tokenize the input string
    WordInfo words[100];
    int word_count = 0;
    char *token = strtok(str, " ");
    while (token != NULL) {
        words[word_count].word = token;
        words[word_count].original_index = word_count;
        word_count++;
        token = strtok(NULL, " ");
    }

    // Find the largest word
    char largest_word[100];
    find_largest_word(words, word_count, largest_word);
    printf("Largest Word: %s\n", largest_word);

    // Sort words by length (ascending or descending)
    int ascending;
    printf("Sort order (1 for Ascending, 0 for Descending): ");
    scanf("%d", &ascending);
    sort_words_by_length(words, word_count, ascending);

    // Print sorted words
    printf("Words sorted by length (%s): ", ascending ? "Ascending" : "Descending");
    for (int i = 0; i < word_count; i++) {
        printf("%s ", words[i].word);
    }
    printf("\n");

    // Detach from shared memory
    if (shmdt(str) == -1) {
        perror("Shared memory detach failed");
    } else {
        printf("Shared memory detached successfully.\n");
    }

    // Destroy the shared memory
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("Failed to destroy shared memory");
    } else {
        printf("Shared memory segment destroyed.\n");
    }

    return 0;
}
