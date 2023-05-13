/* • Scrivere un programma in C che crea un processo
figlio via fork e ne attende la terminazione
• Il processo child :
• invoca una funzione produttore(key) all’interno della quale
alloca e si aggancia ad una porzione di memoria condivisa
per mezzo della chiave key
• scrive un messaggio M passatogli da stdin su memoria
condivisa
• Terminato il processo child, il processo parent:
• invoca una funzione consumatore(key) all’interno della
quale ottiene ed aggancia la stessa memoria condivisa per
mezzo della chiave key
• legge il messaggio M da memoria condivisa
• stampa su stdout il messaggio */

#ifdef __unix__

#include <unistd.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define PAGE_SIZE 4096
#define PAGE_NUMBER 10
#define SIZE PAGE_SIZE * PAGE_NUMBER

#define BUFFER_SIZE 128

char buffer[BUFFER_SIZE]; 

void writer(int shmid)
{
    char *addr = (shmat(shmid, NULL, SHM_W));

    char *addr1 = addr;
    if(addr == (void *)-1){
        handle_error("shmat");
    }

    do{
        printf("Writing to shared memory: ");
        if(!fgets(buffer, BUFFER_SIZE, stdin))
            handle_error("fgets");
        memcpy(addr1, buffer, BUFFER_SIZE); /*copy the buffer's content to addr1 then add an offset to addr1*/
        addr1 += BUFFER_SIZE;
    }while(strcmp(buffer, "exit\n")!= 0 && ((addr1 - addr) < (SIZE - BUFFER_SIZE)));

    exit(0);

}

void reader(int shmid)
{
    char *addr = (shmat(shmid, NULL, SHM_R));

    char *addr1 = addr;
    if(addr == (void *)-1){
        handle_error("shmat");
    }

    while( (strcmp(addr1,"exit") != 0) && ((addr1 - addr) < (SIZE - BUFFER_SIZE))) {
            fprintf(stdout, "%s", addr1);
	    addr1 = addr1 + BUFFER_SIZE;
        }

    exit(0);
}

int main(int argc, char **argv)
{
    int status;
    key_t key = 20;
    long shmid = shmget(key, SIZE, IPC_CREAT|0666);

    if(shmid == -1){
        handle_error("shmget");
    }

   if(fork()){
        wait(&status);
   }

   else{
    writer(shmid);
   }

   if(fork()){
        wait(&status);
   }

   else{
    reader(shmid);
   }

   shmctl(shmid, IPC_RMID, NULL);
    /*FOR DEBUG*/

    // fgets(buffer, sizeof(buffer), stdin);
    // printf("%s\n", buffer);
}