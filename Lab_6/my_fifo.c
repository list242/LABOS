#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUFFER_SIZE 256

int main() {
    pid_t pid;
    char buffer[BUFFER_SIZE];
    if (mkfifo("myfifo", 0666) == -1) {
        perror("mkfifo");
    }
    pid = fork();
    if (pid > 0) {
        int fd;
        time_t parent_time = time(NULL);
        pid_t parent_pid = getpid();
        char message[BUFFER_SIZE];
        snprintf(message, BUFFER_SIZE, "Parent PID: %d, Time: %s", parent_pid, ctime(&parent_time));
        fd = open("myfifo", O_WRONLY);
        write(fd, message, strlen(message) + 1);
        close(fd);
        wait(NULL);
        unlink("myfifo");
    } else if (pid == 0) {
        int fd;
        fd = open("myfifo", O_RDONLY);
        read(fd, buffer, BUFFER_SIZE);
        close(fd);
        sleep(5);
        time_t child_time = time(NULL);
        pid_t child_pid = getpid();
        printf("Child PID: %d, Time: %s", child_pid, ctime(&child_time));
        printf("Received message: %s", buffer);
        exit(EXIT_SUCCESS);
    } else {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    return 0;
}
