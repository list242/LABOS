#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    key_t key = ftok("my_shared_memory", 11);
    if (key < 0) {
        printf("Ошибка при создании ключа\n");
        return 1;
    }

    int shmid = shmget(key, 256, 0666);
    if (shmid < 0) {
        printf("Ошибка при создании сегмента разделяемой памяти\n");
        return 1;
    }

    char *mes = (char*) shmat(shmid, NULL, 0);
    if (mes == (char*)-1) {
        printf("Сегмент не подсоединился\n");
        return 1;
    }

    while (1) {
           time_t now = time(NULL);
           struct tm *tm_info = localtime(&now);
           char time_str[30];
           strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
            
           printf("Текущее время: %s, PID: %d, Принятое сообщение: %s\n", time_str, getpid(), mes);
           sleep(1);
    }


    shmdt(mes);

    return 0;
}

