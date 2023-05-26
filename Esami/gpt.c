#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define MAX_STRING_LENGTH 100
#define MAX_THREADS 10

// Struttura per passare i dati al thread
typedef struct {
    pthread_t thread_id;
    int thread_index;
    char search_string[MAX_STRING_LENGTH];
} ThreadData;

// Dati condivisi tra i thread
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char input_string[MAX_STRING_LENGTH];
int active_threads = 0;

// Funzione eseguita da ogni thread
void *thread_function(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int current_index = data->thread_index;
    int next_index = current_index + 1;

    // Copia la stringa di input locale
    char local_string[MAX_STRING_LENGTH];
    strncpy(local_string, input_string, MAX_STRING_LENGTH);

    // Sostituisce i caratteri nella stringa
    int i;
    for (i = 0; i < strlen(local_string); i++) {
        char *ptr = strchr(data->search_string, local_string[i]);
        if (ptr != NULL) {
            local_string[i] = ' ';
        }
    }

    // Verifica se deve passare la stringa al thread successivo
    if (next_index < active_threads) {
        ThreadData *next_data = (ThreadData *)malloc(sizeof(ThreadData));
        next_data->thread_index = next_index;
        strncpy(next_data->search_string, local_string, MAX_STRING_LENGTH);
        pthread_create(&next_data->thread_id, NULL, thread_function, next_data);
    } else {
        // Ultimo thread, passa la stringa all'OUTPUT thread
        FILE *output_file = fopen("output.txt", "a");
        if (output_file != NULL) {
            fprintf(output_file, "%s\n", local_string);
            fclose(output_file);
        }
    }

    free(data);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    // Verifica il numero di argomenti
    if (argc < 2) {
        printf("Usage: %s <search_string1> <search_string2> ... <search_stringN>\n", argv[0]);
        return 1;
    }

    // Crea i thread in base agli argomenti
    active_threads = argc - 1;
    ThreadData *thread_data[MAX_THREADS];
    int i;
    for (i = 0; i < active_threads; i++) {
        thread_data[i] = (ThreadData *)malloc(sizeof(ThreadData));
        thread_data[i]->thread_index = i;
        strncpy(thread_data[i]->search_string, argv[i + 1], MAX_STRING_LENGTH);
        pthread_create(&thread_data[i]->thread_id, NULL, thread_function, thread_data[i]);
    }

    // Legge indefinitamente le stringhe dallo standard input

    // Attende la terminazione dei thread
    for (i = 0; i < active_threads; i++) {
        pthread_join(thread_data[i]->thread_id, NULL);
        free(thread_data[i]);
    }

    return 0;
}
