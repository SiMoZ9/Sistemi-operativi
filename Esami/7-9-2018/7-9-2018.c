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

#include <math.h>
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

char **mem;

int sem_id1, sem_id2;
struct sembuf sem_op;

FILE *fp;

int num_threads;


void *thread_a(void *arg) {

    long th = (long) arg;
    long i;
    
    sem_op.sem_num = i;
    sem_op.sem_flg = 0;

    while (1) {

        sem_op.sem_op = -1;
        if (semop(sem_id1, &sem_op, 1) == -1)
            handle_error("semop");


        printf("Scrivi qualcosa");
        if (fgets(mem[i], PAGE_SIZE, stdin) == NULL)
            handle_error("fgets");

        sem_op.sem_op = 1;
        if (semop(sem_id2, &sem_op, 1) == -1)
            handle_error("semop");
    }
}

void *thread_b(void *arg) {

    long th = (long) arg;
    long i;
    
    sem_op.sem_num = i;
    sem_op.sem_flg = 0;

    while (1) {

        sem_op.sem_op = -1;
        if (semop(sem_id2, &sem_op, 1) == -1)
            handle_error("semop");

		printf("child worker - thread %d found string %s\n",i,mem[i]);
		fprintf(fp,"%s\n", mem[i]);
		fflush(fp);

        sem_op.sem_op = 1;
        if (semop(sem_id1, &sem_op, 1) == -1)
            handle_error("semop");
    }
}

int main(int argc, char **argv) {

    int shm_id;
    key_t shm_key = 6868;

    long i;
 
    pid_t pid;

    pthread_t tid_a, tid_b;

    mem = (char **)malloc(sizeof(char *) * PAGE_SIZE);

    if (argc != 3)
        quit("Usage: ./prog filename N");

    
#ifdef debug
    printf("filename: %s\n", argv[1]);
    printf("N: %s\n", argv[2]);
#endif


    if ((fp = fopen(argv[1], "w+")) == NULL)
        handle_error("Cannot open file");
    
    num_threads = strtol(argv[2], NULL, 10);

    shm_id = shmget(shm_key, PAGE_SIZE, 0666 | IPC_CREAT);
    if (shm_id == -1)
        handle_error("shmget");





    sem_id1 = semget(IPC_PRIVATE, num_threads, 0666 | IPC_CREAT);
    if (sem_id1 == -1)
        handle_error("semget");

    sem_id2 = semget(IPC_PRIVATE, num_threads, 0666 | IPC_CREAT);
    if (sem_id2 == -1)
        handle_error("semget");


    for (i = 0; i < num_threads; i++) {
        mem[i] = shmat(shm_id, NULL, 0);
        if (mem[i] == (void *) -1)
            handle_error("shmat");
    }

    for (i = 0; i < num_threads; i++) {
        if ( semctl(sem_id1, i, SETVAL, 1) )
            handle_error("semctl");
    }

    for (i = 0; i < num_threads; i++) {
        if ( semctl(sem_id2, i, SETVAL, 0) )
            handle_error("semctl");
    }


  pid = fork();
  

    if (pid > 0) {

        for (i = 0; i < num_threads; i++) {
            if ( pthread_create(&tid_a, NULL, thread_a, (void *)i) != 0 )
                handle_error("pthread_create");
        }

    
        if ( pthread_join(tid_a, NULL) == -1)
            handle_error("pthread_join");

    } else if (pid == 0) {

        for (i = 0; i < num_threads; i++) {
            if ( pthread_create(&tid_b, NULL, thread_b, (void *)i) != 0 )
                handle_error("pthread_create");
        }
    
        if ( pthread_join(tid_b, NULL) == -1)
            handle_error("pthread_join");        
    }

    while(1) pause();

    //semctl(sem_id, SEM_A, IPC_RMID, 0);
   // semctl(sem_id, SEM_B, IPC_RMID, 0);

    for (i = 0; i < num_threads; i++)
        shmdt(mem[i]);

    shmctl(shm_id, IPC_RMID, NULL);

}