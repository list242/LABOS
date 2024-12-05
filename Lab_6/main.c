#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define FIFO_PATH "my_fifo"
#define BUFFER_SIZE 4096

void get_current_time(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

void pipe_example() {
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child process
        close(pipe_fd[1]); // Close write end of pipe
        char received_message[BUFFER_SIZE];
        read(pipe_fd[0], received_message, sizeof(received_message));
        close(pipe_fd[0]);

        // Display current time and received message
        char child_time[64];
        get_current_time(child_time, sizeof(child_time));
        printf("Child process (pipe). Current time: %s\nReceived message: %s\n", child_time, received_message);
        exit(EXIT_SUCCESS);
    } else { // Parent process
        close(pipe_fd[0]); // Close read end of pipe
        char parent_time[64];
        get_current_time(parent_time, sizeof(parent_time));

        char message[BUFFER_SIZE];
        snprintf(message, sizeof(message), "Time: %s, PID: %d", parent_time, getpid());

        // Send message through pipe
        write(pipe_fd[1], message, strlen(message) + 1);
        close(pipe_fd[1]);

        // Wait at least 5 seconds
        sleep(5);

        // Simple wait: manually check if child is still running by checking if it's alive
        if (kill(pid, 0) == -1) {
            printf("Child process has finished.\n");
        }
    }
}

void fifo_example() {
    if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST) {
        perror("FIFO creation failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child process
        char received_message[BUFFER_SIZE];
        int fifo_fd = open(FIFO_PATH, O_RDONLY);
        if (fifo_fd < 0) {
            perror("Error opening FIFO for reading");
            exit(EXIT_FAILURE);
        }
        read(fifo_fd, received_message, sizeof(received_message));
        close(fifo_fd);

        // Display current time and received message
        char child_time[64];
        get_current_time(child_time, sizeof(child_time));
        printf("Child process (FIFO). Current time: %s\nReceived message: %s\n", child_time, received_message);
        unlink(FIFO_PATH); // Remove FIFO
        exit(EXIT_SUCCESS);
    } else { // Parent process
        char parent_time[64];
        get_current_time(parent_time, sizeof(parent_time));

        char message[BUFFER_SIZE];
        snprintf(message, sizeof(message), "Time: %s, PID: %d", parent_time, getpid());

        // Send message through FIFO
        int fifo_fd = open(FIFO_PATH, O_WRONLY);
        if (fifo_fd < 0) {
            perror("Error opening FIFO for writing");
            exit(EXIT_FAILURE);
        }
        write(fifo_fd, message, strlen(message) + 1);
        close(fifo_fd);

        // Wait at least 5 seconds
        sleep(5);

        // Simple wait: manually check if child is still running by checking if it's alive
        if (kill(pid, 0) == -1) {
            printf("Child process has finished.\n");
        }
    }
}

int main() {
    printf("Running pipe example:\n");
    pipe_example();

    printf("\nRunning FIFO example:\n");
    fifo_example();

    return 0;
}
