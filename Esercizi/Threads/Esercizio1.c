/*
• Scrivere un programma per Windows/Unix che
permette al processo principale P di create un
nuovo thread T il cui percorso di esecuzione è
associato alla funzione “thread_function”.
• Il processo principale P ed il nuovo thread T
dovranno stampare ad output una stringa che li
identifichi rispettando l’ordine T→P, senza utilizzare
“WaitForSingleObject”/“pthread_join”, ma
sfruttando un concetto fondamentale che
accomuna tutti i thread di un determinato processo.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define abort(x){puts(x); exit(EXIT_FAILURE);}

//Global variables are shared between threads
int shared_var = 1;

void *thread_function(void *arg)
{
    printf("[2] Thread: %lu is running\n", pthread_self()); 
    sleep(2);
    printf("[3] Thread: %lu is done\n", pthread_self());
    shared_var = 0;
    pthread_exit(NULL);
}

int main()
{
    pthread_t thread;
    int result = pthread_create(&thread, NULL, thread_function, NULL);
    printf("[1] Starting thread\n"); fflush(stdout);
    if(result)
    {
        abort("pthread_create() error");
    } else{
        while(shared_var);
            printf("[4] Parent thread %d is done\n", getpid());
    }

    //printf("Partent process %d", getpid());
    return 0;
}
