#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define NUM_READERS 10
#define BUFFER_SIZE 100

char shared_array[BUFFER_SIZE];
pthread_mutex_t mutex;
int counter = 0;

void *writer_thread(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        counter++;
        snprintf(shared_array, BUFFER_SIZE, "%d", counter);
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
    return NULL;
}
void *reader_thread(void *arg) {
    int thread_num = *((int *)arg);
    free(arg);
    while (1) {
        char local_copy[BUFFER_SIZE];
        strcpy(local_copy, shared_array);
        printf("Читающий поток %d: shared_array = %s\n", thread_num, local_copy);
        sleep(1);
    }
    return NULL;
}
int main() {
    pthread_t writer;
    pthread_t readers[NUM_READERS];
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        printf("Ошибка инициализации мьютекса\n");
        return 1;
    }
    if (pthread_create(&writer, NULL, writer_thread, NULL) != 0) {
        printf("Ошибка создания пишущего потока\n");
        return 1;
    }
    for (int i = 0; i < NUM_READERS; i++) {
        int *arg = malloc(sizeof(*arg));
        if (arg == NULL) {
            printf("Ошибка выделения памяти для аргумента потока\n");
            exit(EXIT_FAILURE);
        }
        *arg = i;
        if (pthread_create(&readers[i], NULL, reader_thread, arg) != 0) {
            printf("Ошибка создания читающего потока %d\n", i);
            return 1;
        }
    }
    sleep(10);
    pthread_cancel(writer);
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_cancel(readers[i]);
    }
    pthread_join(writer, NULL);
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    pthread_mutex_destroy(&mutex);
    return 0;
}
