#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define ARRAY_SIZE 21
#define READER_COUNT 10

char shared_array[ARRAY_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* writer_thread(void* arg) {
    int index = 0;
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&mutex);
        index += snprintf(shared_array + index, ARRAY_SIZE - index, "%d", i);
        if (shared_array[index - 1] == ' ') {
            shared_array[index - 1] = '\0';
        }
        printf("Пишущий поток записал: %d\n", i);
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
    return NULL;
}



void* reader_thread(void* arg) {
    long tid = (long)arg;
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&mutex);
        printf("Читающий поток %ld, tid: %lx, читает: ", tid, pthread_self());
        for (int j = 0; j < ARRAY_SIZE; j++) {
            printf("%c", shared_array[j]);
        }
        printf("\n");
        pthread_mutex_unlock(&mutex);
        sleep(1); 
    }
    return NULL;
}
int main() {
    pthread_t writers, readers[READER_COUNT];

    pthread_create(&writers, NULL, writer_thread, NULL);

    for (long i = 0; i < READER_COUNT; i++) {
        pthread_create(&readers[i], NULL, reader_thread, (void*)i);
    }

    pthread_join(writers, NULL);

    for (int i = 0; i < READER_COUNT; i++) {
        pthread_join(readers[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    return 0;
}
