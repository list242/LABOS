#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>

#define SHM_PATH "shared_memory_example"
#define BUFFER_SIZE 128

typedef struct {
    DWORD sender_pid;  // Используем DWORD для идентификатора процесса в Windows
    char time_str[BUFFER_SIZE];
} shared_data_t;

HANDLE mutex;  // Мьютекс для синхронизации

/** Получение текущего времени в строковом формате */
void get_current_time(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

/** Инициализация разделяемой памяти (используем файл) */
int initialize_shared_memory(HANDLE *shm_fd) {
    *shm_fd = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(shared_data_t), SHM_PATH);
    if (*shm_fd == NULL) {
        printf("Failed to create shared memory (Error: %lu)\n", GetLastError());
        return -1;
    }
    return 0;
}

/** Запись данных в разделяемую память */
int write_to_shared_memory(HANDLE shm_fd, shared_data_t *data) {
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
    return 0;
}

/** Главная функция работы отправителя */
void run_sender(HANDLE shm_fd) {
    shared_data_t data = {0};

    printf("Sender is running. Press Ctrl+C to stop.\n");

    while (1) {
        char current_time[BUFFER_SIZE];
        get_current_time(current_time, BUFFER_SIZE);

        data.sender_pid = GetCurrentProcessId();
        strncpy(data.time_str, current_time, BUFFER_SIZE);

        if (write_to_shared_memory(shm_fd, &data) == -1) {
            break;
        }

        Sleep(1000);  // Задержка 1 секунда
    }
}

/** Очистка ресурсов */
void cleanup_shared_memory(HANDLE shm_fd) {
    CloseHandle(shm_fd);
    CloseHandle(mutex);  // Закрываем мьютекс
}

int main() {
    HANDLE shm_fd;

    // Создание мьютекса
    mutex = CreateMutex(NULL, FALSE, NULL);
    if (mutex == NULL) {
        printf("Failed to create mutex (Error: %lu)\n", GetLastError());
        return EXIT_FAILURE;
    }

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
