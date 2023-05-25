#include <stdio.h>
#include <pthread.h>
#include <string.h>

#define BUFFER_SIZE 256

char buffer[BUFFER_SIZE];
int buffer_filled = 0;
pthread_mutex_t mutex;

void* scrittore(void* arg) {
        pthread_mutex_lock(&mutex);
        printf("Inserisci un input: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer_filled = 1;
        pthread_mutex_unlock(&mutex);
    return NULL;
}

void* lettore(void* arg) {
        pthread_mutex_lock(&mutex);
        if (buffer_filled) {
            printf("Valore letto: %s", buffer);
            buffer_filled = 0;
	}
        pthread_mutex_unlock(&mutex);
    return NULL;
}

int main() {
    pthread_t scrittore_thread, lettore_thread;
    pthread_mutex_init(&mutex, NULL);
    
    pthread_create(&scrittore_thread, NULL, scrittore, NULL);
    pthread_create(&lettore_thread, NULL, lettore, NULL);

    pthread_join(scrittore_thread, NULL);
    pthread_join(lettore_thread, NULL);

    pthread_mutex_destroy(&mutex);

    return 0;
}

