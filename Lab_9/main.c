#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define SHM_SIZE 1024  // Размер общей памяти
#define SEM_KEY 1234   // Ключ семафора
#define SHM_KEY 5678   // Ключ общей памяти

#ifdef __unix__
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
// Функция для установки значения семафора
void set_semaphore_value(int sem_id, int sem_num, int value) {
    if (semctl(sem_id, sem_num, SETVAL, value) == -1) {
        perror("semctl SETVAL");
        exit(EXIT_FAILURE);
    }
}

// Функция для выполнения операций над семафорами
void semaphore_operation(int sem_id, int sem_num, int op) {
    struct sembuf sb = {sem_num, op, 0};
    if (semop(sem_id, &sb, 1) == -1) {
        perror("semop");
        exit(EXIT_FAILURE);
    }
}

// Функция работы производителя
void producer(int shm_id, int sem_id) {
    char *shared_memory = shmat(shm_id, NULL, 0);
    if (shared_memory == (char *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    set_semaphore_value(sem_id, 0, 1); // Семафор "пусто"
    set_semaphore_value(sem_id, 1, 0); // Семафор "данные готовы"

    while (1) {
        semaphore_operation(sem_id, 0, -1); // Ждем, пока "пусто" станет доступным

        printf("Enter a message: ");
        fgets(shared_memory, SHM_SIZE, stdin);

        semaphore_operation(sem_id, 1, 1); // Уведомляем, что данные готовы

        if (strncmp(shared_memory, "exit", 4) == 0) {
            break;
        }
    }

    if (shmdt(shared_memory) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
}

// Функция работы потребителя
void consumer(int shm_id, int sem_id) {
    char *shared_memory = shmat(shm_id, NULL, 0);
    if (shared_memory == (char *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    while (1) {
        semaphore_operation(sem_id, 1, -1); // Ждем, пока "данные готовы" станет доступным

        printf("Received: %s", shared_memory);

        semaphore_operation(sem_id, 0, 1); // Уведомляем, что память снова "пусто"

        if (strncmp(shared_memory, "exit", 4) == 0) {
            break;
        }
    }

    if (shmdt(shared_memory) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
}

// Функция для очистки ресурсов (семафоры и shared memory)
void cleanup_resources(int shm_id, int sem_id) {
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl");
    }
    if (semctl(sem_id, 0, IPC_RMID) == -1) {
        perror("semctl IPC_RMID");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s producer|consumer\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Создание/подключение к сегменту общей памяти
    int shm_id = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget");
        return EXIT_FAILURE;
    }

    // Создание/подключение к семафору
    int sem_id = semget(SEM_KEY, 2, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("semget");
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "producer") == 0) {
        producer(shm_id, sem_id);
        cleanup_resources(shm_id, sem_id);
    } else if (strcmp(argv[1], "consumer") == 0) {
        consumer(shm_id, sem_id);
    } else {
        fprintf(stderr, "Invalid argument. Use 'producer' or 'consumer'.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#endif
