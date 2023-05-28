/*
    SPECIFICATION TO BE IMPLEMENTED: 
    Implementare un'applicazione che riceva in input tramite argv[] il 
    nome di un file F ed una stringa indicante un valore numerico N maggiore
    o uguale ad 1.
    L'applicazione, una volta lanciata dovra' creare il file F ed attivare 
    N thread. Inoltre, l'applicazione dovra' anche attivare un processo 
    figlio, in cui vengano attivati altri N thread. 
    I due processi che risulteranno attivi verranno per comodita' identificati
    come A (il padre) e B (il figlio) nella successiva descrizione.

    Ciascun thread del processo A leggera' stringhe da standard input. 
    Ogni stringa letta dovra' essere comunicata al corrispettivo thread 
    del processo B tramite memoria condivisa, e questo la scrivera' su una 
    nuova linea del file F. Per semplicita' si assuma che ogni stringa non
    ecceda la taglia di 4KB. 

    L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
    WinAPI) in modo tale che quando il processo A venga colpito esso dovra' 
    inviare la stessa segnalazione verso il processo B. Se invece ad essere 
    colpito e' il processo B, questo dovra' riversare su standard output il 
    contenuto corrente del file F.

    Qalora non vi sia immissione di input, l'applicazione dovra' utilizzare 
    non piu' del 5% della capacita' di lavoro della CPU. 
*/

#ifndef __unix__

#error "Cannot compile non non Unix systems"

#else

#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define quit(msg) \
    do { fprintf(stdout, "%s\n", msg); exit(EXIT_FAILURE); } while (0)

#define PAGE_SIZE 4096
#define SEM_A 0
#define SEM_B 1

pthread_mutex_t *ready;
pthread_mutex_t *done;

char *shm_addr;

struct sembuf sem_op;

FILE *fp;

void my_wait(int sem_id, int sem_num) {
    sem_op.sem_num = sem_num;
    sem_op.sem_op = -1;
    sem_op.sem_flg = 0;

    if (semop(sem_id, &sem_op, 1) == -1)
        handle_error("semop");
}

void my_post(int sem_id, int sem_num) {
    sem_op.sem_num = sem_num;
    sem_op.sem_op = 1;
    sem_op.sem_flg = 0;
    
    if (semop(sem_id, &sem_op, 1) == -1)
        handle_error("semop");
}

void *thread_a(void *arg) {

    pthread_mutex_lock(ready);

    if ( fgets((char *)arg, PAGE_SIZE, stdin) == NULL)
        handle_error("fgets");

    pthread_mutex_unlock(done);
}

void *thread_b(void *arg) {

    pthread_mutex_lock(done);

    if ( fprintf(fp, "%s\n", (char *)arg) == -1 )
        handle_error("fprintf");

    pthread_mutex_unlock(ready);
}

int main(int argc, char **argv) {

    int shm_id, sem_id;
    key_t shm_key = 6868, sem_key = 3434;
    long num_threads;

    long i;

    pthread_t tid_a;
    pthread_t tid_b;

    if (argc != 2)
        quit("Usage: filename N");

    num_threads = strtol(argv[2], NULL, 10);

    if ((fp = fopen(argv[2], "w+")) == NULL)
        handle_error("Cannot open file");

    

    shm_id = shmget(shm_key, PAGE_SIZE, 0666 | IPC_CREAT);
    if (shm_id == -1)
        handle_error("shmget");

    shm_addr = shmat(shm_id, NULL, 0);
    if (shm_addr == (void *) -1)
        handle_error("shmat");



    sem_id = semget(sem_key, 2, 0666 | IPC_CREAT);
    if (sem_id == -1)
        handle_error("semget");

    if ( semctl(sem_id, SEM_A, SETVAL, 0) == -1 || semctl(sem_id, SEM_B, SETVAL, 1) == -1 )
        handle_error("semctl");



    for (i = 0; i < num_threads; i++) {       
    }
}