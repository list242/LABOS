#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

int main() {
    const char* path = "./shm";
    key_t key = ftok(path, 11);
    if (key == -1) {
        fprintf(stderr, "Ошибка: не удалось создать ключ. Возможно, отправитель не запущен.\n");
        return 1;
    }
    int shmid = shmget(key, 128, 0666);
    if (shmid == -1) {
        fprintf(stderr, "Ошибка: разделяемая память не открыта или отправитель не запущен.\n");
        return 1;
    }
    char *mas = shmat(shmid, NULL, 0);
    if (mas == (char*)-1) {
        fprintf(stderr, "Ошибка: не удалось присоединиться к разделяемой памяти.\n");
        return 1;
    }
    while (1) {
        time_t time2 = time(NULL);
        struct tm *time_info = localtime(&time2);
        if (time_info == NULL) {
            fprintf(stderr, "Ошибка получения локального времени.\n");
            break;
        }
        printf("Приемник: PID=%u, текущее время: %02d:%02d:%02d\n", 
               (unsigned)getpid(),
               time_info->tm_hour, 
               time_info->tm_min, 
               time_info->tm_sec);
        printf("Данные от отправителя: %s", mas);
        printf("_____________________\n");
        sleep(1);
    }
    shmdt(mas);
    return 0;
}
