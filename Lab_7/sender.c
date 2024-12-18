#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

static const char* path = "./shm";
static int shmid = -1;
static char* mas = NULL;

void cleanup() {
    if (mas != NULL && mas != (char*)-1) {
        shmdt(mas);
        mas = NULL;
    }
    if (shmid != -1) {
        shmctl(shmid, IPC_RMID, NULL);
        shmid = -1;
    }
    unlink(path);
}
void signal_handler(int sig) {
    cleanup();
    _exit(0);
}
int main() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    int path_fd = open(path, O_CREAT | O_EXCL, 0666);
    if (path_fd == -1) {
        fprintf(stderr, "Ошибка: уже запущен процесс-передатчик данных (файл shm уже существует).\n");
        return 1;
    }
    close(path_fd);
    key_t key = ftok(path, 11);
    if (key == -1) {
        fprintf(stderr, "Ошибка при создании ключа общей памяти.\n");
        cleanup();
        return 1;
    }
    shmid = shmget(key, 128, IPC_CREAT | IPC_EXCL | 0666);
    if (shmid == -1) {
        fprintf(stderr, "Ошибка: сегмент общей памяти уже существует или не может быть создан.\n");
        cleanup();
        return 1;
    }
    mas = shmat(shmid, NULL, 0);
    if (mas == (char*)-1) {
        fprintf(stderr, "Ошибка присоединения к общей памяти.\n");
        cleanup();
        return 1;
    }
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
        sleep(1);
    }
    cleanup();
    return 0;
}
