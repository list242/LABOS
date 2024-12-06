#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#ifdef __unix__
#include <sys/wait.h>
#endif
#define ARRAY_SIZE 10       
#define NUM_READERS 10      

char shared_array[ARRAY_SIZE] = {0};

atomic_int write_count = 0;

atomic_flag lock = ATOMIC_FLAG_INIT;
void lock_mutex() {
    while (atomic_flag_test_and_set(&lock)) {
        
        usleep(100);
    }
}

// Функция для разблокировки (имитация мьютекса)
void unlock_mutex() {
    atomic_flag_clear(&lock);
}

// Пишущий процесс
void writer() {
    while (1) {
        lock_mutex();

        // Запись в массив (пишущий процесс записывает номер записи)
        shared_array[write_count % ARRAY_SIZE] = (write_count % 10) + '0'; // Записываем цифру как символ
        printf("Writer: Written %d at index %d\n", write_count, write_count % ARRAY_SIZE);
        write_count++;

        unlock_mutex();

        // Задержка для симуляции работы
        usleep(500000); // 0.5 секунды
    }
}

// Читающий процесс
void reader(int tid) {
    while (1) {
        lock_mutex();

        // Чтение текущего состояния массива
        printf("Reader %d: Current array state: ", tid);
        for (int i = 0; i < ARRAY_SIZE; i++) {
            printf("%c ", shared_array[i] ? shared_array[i] : '_'); // Пустые элементы отображаются как '_'
        }
        printf("\n");

        unlock_mutex();

        // Задержка для симуляции работы
        usleep(500000); // 0.5 секунды
    }
}

int main() {
    pid_t writer_pid;
    pid_t reader_pids[NUM_READERS];

    // Создаем пишущий процесс
    writer_pid = fork();
    if (writer_pid == -1) {
        perror("Failed to create writer process");
        return EXIT_FAILURE;
    } else if (writer_pid == 0) {
        writer(); // Запуск функции писателя в дочернем процессе
        exit(EXIT_SUCCESS);
    }

    // Создаем читающие процессы
    for (int i = 0; i < NUM_READERS; i++) {
        reader_pids[i] = fork();
        if (reader_pids[i] == -1) {
            perror("Failed to create reader process");
            return EXIT_FAILURE;
        } else if (reader_pids[i] == 0) {
            reader(i); // Запуск функции читателя в дочернем процессе
            exit(EXIT_SUCCESS);
        }
    }

    wait(NULL);
    return EXIT_SUCCESS;
}
