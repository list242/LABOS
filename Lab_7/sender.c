#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>

const char *shm_name = "./my_shared_memory";

int main() {

    open(shm_name, O_CREAT |  O_EXCL | 0);
    key_t key = ftok(shm_name, 11);
    if (key < 0) {
        perror("Ошибка при генерации ключа");
        return 1;
    }

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

    char *mes = (char*) shmat(shmid, NULL, 0);
    if (mes == (char*)-1) {
        perror("Ошибка при подключении к сегменту");
        return 1;
    }

    while (1) {
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
       
        strftime(mes, 256, "%Y-%m-%d %H:%M:%S", tm_info);
        sprintf(mes + strlen(mes), " PID: %d", getpid());
        
        sleep(3);

    }


    shmdt(mes);
    shmctl(shmid, IPC_RMID, NULL);
    remove(shm_name);

    return 0;
}
