#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

struct sembuf sem_lock = {0, -1, 0};
struct sembuf sem_open = {0, 1, 0};

const char *shm_name = "my_shared_memory";
int shmid;
int semid;

void cleanup(int signum) {
    if (shmid != -1) {
        shmctl(shmid, IPC_RMID, NULL);
    }
    if (semid != -1) {
        semctl(semid, 0, IPC_RMID);
    }
    unlink(shm_name);
    exit(0);
}

int main() {
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    int fd = open(shm_name, O_CREAT |  O_EXCL | 0666);
    key_t key = ftok(shm_name, 11);
    if (key < 0) {
        perror("Ошибка при генерации ключа");
        return 1;
    }
    close(fd);

    int shmid = shmget(key, 256, 0666 | IPC_CREAT);
    if (shmid < 0) {
        perror("Ошибка при создании сегмента");
        return 1;
    }

    struct shmid_ds shmid_ds_info;
    if (shmctl(shmid, IPC_STAT, &shmid_ds_info) < 0) {
        perror("Ошибка при получении информации о сегменте");
        return 1;
    }

    if (shmid_ds_info.shm_nattch >= 1) {
        perror("Уже существует");
        exit(1); 
    }

    int semid = semget(key, 1, IPC_CREAT | 0666);
    if (semid < 0) {
        perror("Ошибка при создании семафора");
        return 1;
    }

    semctl(semid, 0, SETVAL, 1);

    char *mes = (char*) shmat(shmid, NULL, 0);
    if (mes == (char*)-1) {
        perror("Ошибка при подключении к сегменту");
        return 1;
    }

    while (1) {
        semop(semid, &sem_lock, 1);
	
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        
        strftime(mes, 256, "%Y-%m-%d %H:%M:%S", tm_info);
        sprintf(mes + strlen(mes), " PID: %d", getpid());
        semop(semid, &sem_open, 1);
	    sleep(3);
    }

    shmdt(mes);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);

    return 0;
}
