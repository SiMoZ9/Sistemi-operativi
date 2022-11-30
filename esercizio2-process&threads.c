#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

void *thread_func(void *p)
{
    printf("Stampo: ");
    fflush(stdout);
    for(int i = 0; i < 20; i++)
    {
        printf(" %d ", i);
        sleep(1);
        fflush(stdout);
    }

    sleep(2);

    printf("Processo terminato\n");
    pthread_exit(NULL);
    return NULL;
}

int main()
{
    pthread_t t;
    if(pthread_create(&t, NULL, thread_func, NULL))
    {
        printf("Errore nella creazione del thread\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    if(pthread_join(t, NULL))
    {
        printf("Thread non sincronizzato\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

}