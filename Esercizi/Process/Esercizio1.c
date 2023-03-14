/*
Scrivere un programma in C che prende
inizialmente una stringa da input (può contenere
anche spazi bianchi) e la salva in un buffer

• fork-are un processo figlio che manda in stampa la
stessa stringa acquisita dal processo padre.

• Il processo padre termina solo dopo che il processo
figlio ha terminato (verificare che tale ordine è
rispettato stampando i PID dei processi).
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER 1024
#define ERROR(x){puts(x); exit(1);}

char *getString()
{
    char *mex;
    if(fgets(mex, BUFFER, stdin) == NULL)
        ERROR("fgets() error");
    
    mex[strlen(mex) + 1] = '\0';
    return mex;
}

int main(int argc, char **argv)
{
    pid_t child;
    int status;

    printf("Put a string: ");
    char *mex = getString();
    child = fork();

    if(child == 0)
        printf("Hi i'm child, that's my PID %u and this is the string %s\n", getpid(), mex);

    if(child > 0){
        printf("Hi i'm father and, that's my PID %u and i'm gonna waiting my child's death :)\n", getpid());
        wait(&status);
        printf("Hi i'm father and, that's my PID %u, my child is dead and i'm gonna die too", getpid());
    }
    exit(0);
}
