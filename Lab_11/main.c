#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define NUM_READERS 10
#define BUFFER_SIZE 100

char shared_array[BUFFER_SIZE];
int counter = 0;

pthread_rwlock_t rwlock;

void *writer_thread(void *arg);
void *reader_thread(void *arg);

int init_rwlock();
void create_writer_thread(pthread_t *writer);
void create_reader_threads(pthread_t readers[NUM_READERS]);
void join_threads(pthread_t writer, pthread_t readers[NUM_READERS]);
void cleanup();

// Main function
int main() {
    pthread_t writer;
    pthread_t readers[NUM_READERS];

    // Initialize resources
    if (init_rwlock() != 0) {
        return 1;
    }

    // Create writer and reader threads
    create_writer_thread(&writer);
    create_reader_threads(readers);

    // Let the threads run for a while
    sleep(10);

    // Cancel threads after 10 seconds
    pthread_cancel(writer);
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_cancel(readers[i]);
    }

    // Wait for threads to finish
    join_threads(writer, readers);

    // Cleanup resources
    cleanup();

    return 0;
}

// Initialize the read-write lock
int init_rwlock() {
    if (pthread_rwlock_init(&rwlock, NULL) != 0) {
        printf("Ошибка инициализации RW-блокировки\n");
        return 1;
    }
    return 0;
}

// Create the writer thread
void create_writer_thread(pthread_t *writer) {
    if (pthread_create(writer, NULL, writer_thread, NULL) != 0) {
        printf("Ошибка создания пишущего потока\n");
        exit(1);
    }
}

// Create the reader threads
void create_reader_threads(pthread_t readers[NUM_READERS]) {
    for (int i = 0; i < NUM_READERS; i++) {
        int *arg = malloc(sizeof(*arg));
        if (arg == NULL) {
            printf("Ошибка выделения памяти для аргумента потока\n");
            exit(EXIT_FAILURE);
        }
        *arg = i;
        if (pthread_create(&readers[i], NULL, reader_thread, arg) != 0) {
            printf("Ошибка создания читающего потока %d\n", i);
            exit(1);
        }
    }
}

// Join writer and reader threads
void join_threads(pthread_t writer, pthread_t readers[NUM_READERS]) {
    pthread_join(writer, NULL);
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
}

// Cleanup resources
void cleanup() {
    pthread_rwlock_destroy(&rwlock);
}

// Writer thread function
void *writer_thread(void *arg) {
    while (1) {
        pthread_rwlock_wrlock(&rwlock);
        counter++;
        snprintf(shared_array, BUFFER_SIZE, "Запись №%d", counter);
        pthread_rwlock_unlock(&rwlock);
        sleep(1);
    }
    return NULL;
}

// Reader thread function
void *reader_thread(void *arg) {
    int thread_num = *((int *)arg);
    free(arg);

    while (1) {
        pthread_rwlock_rdlock(&rwlock);
        char local_copy[BUFFER_SIZE];
        strncpy(local_copy, shared_array, BUFFER_SIZE - 1);
        local_copy[BUFFER_SIZE - 1] = '\0';
        int local_counter = counter;
        pthread_rwlock_unlock(&rwlock);
        printf("Читающий поток %d: shared_array = %s, counter = %d\n", thread_num, local_copy, local_counter);
        sleep(1);
    }
    return NULL;
}
