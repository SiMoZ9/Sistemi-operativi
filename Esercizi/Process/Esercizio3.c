/*
    Esercizio 3
    • Scrivere un programma in C che prende
    inizialmente N (a piacere) stringhe rappresentanti
    N directory corrette

    • fork-a quindi N processi che andranno ad eseguire
    il comando ls su una directory differente.

    • Il processo padre termina dopo i processi figli
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

#define BUFFER 1024

#define ERROR(x){puts(x); exit(EXIT_FAILURE);}

char **strings;

void getStrings()
{
    strings = (char **)malloc(sizeof(char *) * BUFFER);
    *strings = (char *)malloc(sizeof(char) * BUFFER);

    int i;
    int n = 4;
    char *string;

    for(i = 0; i < n; i++)
    {   
        printf("Path %d: ", i+1);
        scanf("%ms", &strings[i]);
    }
}

void printArray()
{
    for(int i = 0; i < 4; i++)
        printf("%s\n", strings[i]);
}


int main(void)
{
    pid_t pid;
    int status, forked;
    
    getStrings();

    for(int i = 0; i < 4; i++)
    {
        pid = fork();
        if(pid < 0)
            ERROR("fork() error");

        if(pid == 0)
        {
            execlp("ls", "ls", strings[i], NULL);
            printf("\n\n");
        }
        else
            forked += 1;
    }

    for(int i = 0; i < forked; i++)
    {
        wait(&status);
    }
}