/*
Scrivere un programma in C che prende
inizialmente una stringa da input (può contenere
anche spazi bianchi) e la salva in un buffer

• fork-are 2 processi figli che contribuiscono a
stampare la stringa inversa della stringa acquisita
dal processo padre.

• Il processo padre termina solo dopo che i processi
figli hanno terminato.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER 256
#define ERROR(x){puts(x); exit(1);}
#define CHILD 2

char mex[BUFFER];


char *getString()
{
    if(fgets(mex, BUFFER, stdin) == NULL)
        ERROR("fgets() error");
    
    mex[strlen(mex)-1] = '\0';
    return mex;
}

void reverseString(char *mex){
    char *reversedString = mex;
    char *end = reversedString + strlen(mex) - 1;
    char temp;

    while(end > reversedString){
        temp = *reversedString;
        *reversedString = *end;
        *end = temp;

        ++reversedString;
        --end;
    }
}

int main(void)
{
    pid_t pid;
    int i, status, forked;

    printf("Put a string: ");
    char *string = getString();

    reverseString(string);

    printf("That's father's id: %u\n", getpid());
    fflush(stdout);

    for(i = 0; i < CHILD; i++){
        pid = fork();
        if(pid < 0)
            ERROR("fork() error");

        if(pid == 0)
        {
            printf("PID %u reads string: %s\n", getpid(), mex);
            fflush(stdout);
            exit(0);
        }

        else
            forked += 1;
    }

    for(int j = 0; j < forked; j++)
    {
        wait(&status);
    }
}