#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define NUM_READERS 10
#define BUFFER_SIZE 100

char shared_array[BUFFER_SIZE];
int counter = 0;

pthread_mutex_t mutex;
pthread_cond_t cond;

void *writer_thread(void *arg);
void *reader_thread(void *arg);

int init_mutex_and_cond();
void create_writer_thread(pthread_t *writer);
void create_reader_threads(pthread_t readers[NUM_READERS]);
void join_threads(pthread_t writer, pthread_t readers[NUM_READERS]);
void cleanup();

int main() {
    pthread_t writer;
    pthread_t readers[NUM_READERS];

    if (init_mutex_and_cond() != 0) {
        return 1;
    }

    create_writer_thread(&writer);
    create_reader_threads(readers);

    // Let the threads run for a while
    sleep(10);

    // Cancel writer and reader threads
    pthread_cancel(writer);
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_cancel(readers[i]);
    }

    join_threads(writer, readers);
    cleanup();

    return 0;
}

// Function to initialize mutex and condition variable
int init_mutex_and_cond() {
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        printf("Ошибка инициализации мьютекса\n");
        return 1;
    }
    if (pthread_cond_init(&cond, NULL) != 0) {
        printf("Ошибка инициализации условной переменной\n");
        return 1;
    }
    return 0;
}

// Function to create the writer thread
void create_writer_thread(pthread_t *writer) {
    if (pthread_create(writer, NULL, writer_thread, NULL) != 0) {
        printf("Ошибка создания пишущего потока\n");
        exit(1);
    }
}

// Function to create the reader threads
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

// Function to join writer and reader threads
void join_threads(pthread_t writer, pthread_t readers[NUM_READERS]) {
    pthread_join(writer, NULL);
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
}

// Function to clean up mutex, condition variable, and other resources
void cleanup() {
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}

// Writer thread function
void *writer_thread(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        counter++;
        snprintf(shared_array, BUFFER_SIZE, "Запись №%d", counter);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
    return NULL;
}

// Reader thread function
void *reader_thread(void *arg) {
    int thread_num = *((int *)arg);
    free(arg);
    int last_read_value = 0;
    while (1) {
        pthread_mutex_lock(&mutex);
        while (counter <= last_read_value) {
            pthread_cond_wait(&cond, &mutex);
        }
        char local_copy[BUFFER_SIZE];
        strncpy(local_copy, shared_array, BUFFER_SIZE - 1);
        local_copy[BUFFER_SIZE - 1] = '\0';
        last_read_value = counter; 
        pthread_mutex_unlock(&mutex);
        printf("Читающий поток %d: shared_array = %s\n", thread_num, local_copy);
    }
    return NULL;
}
