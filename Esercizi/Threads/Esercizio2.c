/*
    • Scrivere un programma per Unix che sia in grado di
    generare due thread T1 e T2, tali per cui:
    • T1 chiede all’utente di inserire una messaggio da
    tastiera
    • T2 scrive il messaggio ottenuto dall’utente a
    schermo
    • non devono essere usate variabili globali.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define SIZE 128
#ifndef ABORT
    #define abort(x){puts(x);exit(EXIT_FAILURE);}
#endif

#ifndef TH_ABORT
    #define TH_ABORT(x, ex){puts(x); pthread_exit(ex);}
#endif

void *writer(void *arg)
{
    char *msg = (char *)malloc(sizeof(char)*SIZE);
    printf("Writer wants a message: ");
    if(fgets(msg, SIZE, stdin) == NULL)
        abort("fgets() failed");

    pthread_exit((void *)msg);
}

void *reader(void *arg)
{
    char *msg = (char *)arg;
    if(msg==NULL)
        TH_ABORT("reader() failed", (void *) 1);
    printf("Reader reads %s", msg);
    pthread_exit((void *)0);
}

int main(int argc, char *argv[])
{
    pthread_t thread;    
    void *msg;
    void *ret; 

    if(pthread_create(&thread, NULL, writer, NULL))
        abort("pthread_create() failed");
    if(pthread_join(thread, &msg))
        abort("pthread_join() writer failed");


    if(pthread_create(&thread, NULL, reader, msg))
        abort("pthread_create() failed");
    if(pthread_join(thread, &ret))
        abort("pthread_join() reader failed");
    
    return 0;
}
