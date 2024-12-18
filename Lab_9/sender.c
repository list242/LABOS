#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

int shmid = -1;
int semid = -1;
char* mas = (char*)-1;
const char* path = "./shm";
int path_fd = -1;
void cleanup() {
    if (mas != (char*)-1) {
        shmdt(mas);
        mas = (char*)-1;
    }
    if (shmid != -1) {
        shmctl(shmid, IPC_RMID, NULL);
        shmid = -1;
    }
    if (semid != -1) {
        semctl(semid, 0, IPC_RMID);
        semid = -1;
    }
    unlink(path);
    if (path_fd != -1) {
        close(path_fd);
        path_fd = -1;
    }
}
void sigint_handler(int signo) {
    cleanup();
    exit(0);
}
int main() {
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);
    path_fd = open(path, O_CREAT | O_EXCL | O_RDWR, 0666);
    if (path_fd == -1) {
        fprintf(stderr, "Ошибка: уже существует процесс, передающий данные (файл shm уже существует).\n");
        return 1;
    }
    key_t key = ftok(path, 11);
    if (key == -1) {
        fprintf(stderr, "Ошибка: невозможно создать ключ ftok.\n");
        cleanup();
        return 1;
    }
    shmid = shmget(key, 128, IPC_CREAT | IPC_EXCL | 0666);
    if (shmid == -1) {
        fprintf(stderr, "Ошибка: невозможно создать сегмент памяти.\n");
        cleanup();
        return 1;
    }
    semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (semid == -1) {
        fprintf(stderr, "Ошибка: невозможно создать семафор.\n");
        cleanup();
        return 1;
    }
    if (semctl(semid, 0, SETVAL, 0) == -1) {
        fprintf(stderr, "Ошибка: невозможно инициализировать семафор.\n");
        cleanup();
        return 1;
    }
    mas = shmat(shmid, NULL, 0);
    if (mas == (char*)-1) {
        fprintf(stderr, "Ошибка присоединения к общей памяти.\n");
        cleanup();
        return 1;
    }
    struct sembuf unlock = {0, 1, 0};
    while (1) {
        time_t rawtime = time(NULL);
        struct tm *time_info = localtime(&rawtime);
        if (time_info == NULL) {
            fprintf(stderr, "Ошибка получения локального времени.\n");
            break;
        }
        char time_str[64];
        strftime(time_str, sizeof(time_str), "%H:%M:%S", time_info);
        snprintf(mas, 128, "[Передача] Процесс PID=%u, время: %s\n", (unsigned)getpid(), time_str);
        if (semop(semid, &unlock, 1) == -1) {
            fprintf(stderr, "Ошибка при освобождении семафора.\n");
            break;
        }
        sleep(1);
    }
    cleanup();
    return 0;
}
