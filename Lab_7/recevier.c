#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>  // Для pid_t
#endif

#define SHM_PATH "shared_memory_example"
#define BUFFER_SIZE 128

// В зависимости от операционной системы используем разные типы для идентификатора процесса
#ifdef _WIN32
typedef DWORD pid_t;  // Используем DWORD для Windows
#endif

typedef struct {
    pid_t sender_pid;  // Используем pid_t для UNIX, DWORD для Windows
    char time_str[BUFFER_SIZE];
} shared_data_t;

#ifdef _WIN32
HANDLE mutex;  // Мьютекс для синхронизации
#else
sem_t *mutex;  // Семфор для синхронизации
#endif

/** Получение текущего времени в строковом формате */
void get_current_time(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

#ifdef _WIN32
/** Инициализация разделяемой памяти для Windows */
int initialize_shared_memory(HANDLE *shm_fd) {
    *shm_fd = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(shared_data_t), SHM_PATH);
    if (*shm_fd == NULL) {
        printf("Failed to create shared memory (Error: %lu)\n", GetLastError());
        return -1;
    }
    return 0;
}
#else
/** Инициализация разделяемой памяти для UNIX */
int initialize_shared_memory(int *shm_fd) {
    *shm_fd = shm_open(SHM_PATH, O_CREAT | O_RDWR, 0666);
    if (*shm_fd == -1) {
        perror("Failed to open shared memory");
        return -1;
    }
    if (ftruncate(*shm_fd, sizeof(shared_data_t)) == -1) {
        perror("Failed to set size of shared memory");
        return -1;
    }
    return 0;
}
#endif

/** Запись данных в разделяемую память */
int write_to_shared_memory(void *shm_fd, shared_data_t *data) {
#ifdef _WIN32
    // Используем мьютекс для синхронизации
    WaitForSingleObject(mutex, INFINITE);

    LPVOID ptr = MapViewOfFile(shm_fd, FILE_MAP_WRITE, 0, 0, sizeof(shared_data_t));
    if (ptr == NULL) {
        printf("Failed to map shared memory (Error: %lu)\n", GetLastError());
        ReleaseMutex(mutex);
        return -1;
    }

    memcpy(ptr, data, sizeof(shared_data_t));

    UnmapViewOfFile(ptr);
    ReleaseMutex(mutex);
#else
    // Используем семафор для синхронизации
    sem_wait(mutex);

    void *ptr = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, *(int *)shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("Failed to map shared memory");
        sem_post(mutex);
        return -1;
    }

    memcpy(ptr, data, sizeof(shared_data_t));
    munmap(ptr, sizeof(shared_data_t));

    sem_post(mutex);
#endif
    return 0;
}

/** Главная функция работы отправителя */
void run_sender(void *shm_fd) {
    shared_data_t data = {0};

    printf("Sender is running. Press Ctrl+C to stop.\n");

    while (1) {
        char current_time[BUFFER_SIZE];
        get_current_time(current_time, BUFFER_SIZE);

        data.sender_pid = getpid();
        strncpy(data.time_str, current_time, BUFFER_SIZE);

        if (write_to_shared_memory(shm_fd, &data) == -1) {
            break;
        }

        sleep(1);  // Задержка 1 секунда
    }
}

/** Очистка ресурсов */
void cleanup_shared_memory(void *shm_fd) {
#ifdef _WIN32
    CloseHandle(shm_fd);
    CloseHandle(mutex);  // Закрываем мьютекс
#else
    close(*(int *)shm_fd);
    sem_close(mutex);  // Закрываем семафор
#endif
}

int main() {
    void *shm_fd;
    
#ifdef _WIN32
    // Создание мьютекса для Windows
    mutex = CreateMutex(NULL, FALSE, NULL);
    if (mutex == NULL) {
        printf("Failed to create mutex (Error: %lu)\n", GetLastError());
        return EXIT_FAILURE;
    }
#else
    // Создание семафора для UNIX
    mutex = sem_open("/shared_memory_mutex", O_CREAT, 0666, 1);
    if (mutex == SEM_FAILED) {
        perror("Failed to create semaphore");
        return EXIT_FAILURE;
    }
#endif

    // Инициализация разделяемой памяти
    if (initialize_shared_memory(&shm_fd) == -1) {
        return EXIT_FAILURE;
    }

    // Запуск основной логики отправителя
    run_sender(shm_fd);

    // Очистка ресурсов
    cleanup_shared_memory(shm_fd);

    return EXIT_SUCCESS;
}
