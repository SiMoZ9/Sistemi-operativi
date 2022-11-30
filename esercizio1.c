#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/time.h>

void *thread_function(void *arg)
{
    printf("Pid: %d\n Thread_id: %d\n", getpid(), pthread_self());
    fflush(stdout);
    sleep(2);
    printf("So morto\n");
    fflush(stdout);
    pthread_exit(NULL);
}

int main()
{
    pthread_t thread;
    if(pthread_create(&thread, NULL, thread_function, NULL))
    {
        printf("Errore nella creazione del thread\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    if(pthread_join(thread, NULL))
    {
        printf("Errore nella sincronizzazione del thread");
    }
}
