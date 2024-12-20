#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

struct sembuf sem_lock = {0, -1, 0};
struct sembuf sem_open = {0, 1, 0};

int main() {
    key_t key = ftok("my_shared_memory", 11);
    if (key < 0) {
        perror("Ошибка при генерации ключа");
        return 1;
    }

    int shmid = shmget(key, 256, 0666);
    if (shmid < 0) {
        perror("Ошибка при получении сегмента");
        return 1;
    }

    int semid = semget(key, 1, 0666);
    if (semid < 0) {
        perror("Ошибка при получении семафора");
        return 1;
    }

    char *mes = (char*) shmat(shmid, NULL, 0);
    if (mes == (char*)-1) {
        perror("Ошибка при подключении к сегменту");
        return 1;
    }

    while (1) {
        semop(semid, &sem_lock, 1);
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char time_str[30];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
        printf("Текущее время: %s, PID: %d, Принятое сообщение: %s\n", time_str, getpid(), mes);
        semop(semid, &sem_open, 1);
	    sleep(1);

    }

    shmdt(mes);

    
    return 0;
}

