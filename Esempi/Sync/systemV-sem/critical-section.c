#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUF_SIZE 1024

char buffer[BUF_SIZE];

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

void *reader(void *arg){
    for(int i = 0; i < (sizeof(buffer)/sizeof(char *)); i++)
        printf("%s\n", buffer);
}

void *writer(void *arg)
{
    do{
        printf("Put something: ");
        if(fgets(buffer, BUF_SIZE, stdin) == NULL)
            handle_error("fgets");

    }while(strcmp(buffer, "exit\n")!= 0);
}

int main(void){
    pthread_t prod, cons;
    pthread_create(&prod, NULL, writer, NULL);
    sleep(1);
    pthread_create(&cons, NULL, reader, NULL);

    pause();
}