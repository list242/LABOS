#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>

int main() {
    const char* path = "./shm";
    key_t key = ftok(path, 11);
    if (key == -1) {
        fprintf(stderr, "Ошибка: невозможно создать ключ ftok. Убедитесь, что передающая программа запущена.\n");
        return 1;
    }
    int shmid = shmget(key, 128, 0666);
    int semid = semget(key, 1, 0666);
    if (shmid == -1 || semid == -1) {
        fprintf(stderr, "Ошибка: невозможно подключиться к общей памяти или семафору. Убедитесь, что передающая программа запущена.\n");
        return 1;
    }
    char *mas = shmat(shmid, NULL, 0);
    if (mas == (char*)-1) {
        fprintf(stderr, "Ошибка присоединения к общей памяти.\n");
        return 1;
    }
    struct sembuf lock = {0, -1, 0};
    while (1) {
        if (semop(semid, &lock, 1) == -1) {
            fprintf(stderr, "Ошибка при ожидании семафора. Возможно, передающая программа завершилась.\n");
            break;
        }
        time_t rawtime = time(NULL);
        struct tm *time_info = localtime(&rawtime);
        if (time_info == NULL) {
            fprintf(stderr, "Ошибка получения локального времени.\n");
            break;
        }
        char time_str[64];
        strftime(time_str, sizeof(time_str), "%H:%M:%S", time_info);
        printf("[Приём] Процесс PID=%u, текущее время: %s\n", (unsigned)getpid(), time_str);
        printf("Получено сообщение: %s", mas);
        printf("-----------------------------------------\n");
        sleep(1);
    }
    shmdt(mas);
    return 0;
}
