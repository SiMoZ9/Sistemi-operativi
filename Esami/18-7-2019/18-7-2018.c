/*
SPECIFICATION TO BE IMPLEMENTED:
Implementare un programma che riceva in input tramite argv[] i pathname 
associati ad N file, con N maggiore o uguale ad 1. 

Per ognuno di questi file generi un processo che legga tutte le stringhe contenute in quel file
e le scriva in un'area di memoria condivisa con il processo padre. 

Si supponga per semplicita' che lo spazio necessario a memorizzare le stringhe
di ognuno di tali file non ecceda 4KB.

Il processo padre dovra' attendere che tutti i figli abbiano scritto in 
memoria il file a loro associato, e successivamente dovra' entrare in pausa
indefinita.

D'altro canto, ogni figlio dopo aver scritto il contenuto del file nell'area 
di memoria condivisa con il padre entrera' in pausa indefinita.

L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che quando il processo padre venga colpito da esso dovra' 
stampare a terminale il contenuto corrente di tutte le aree di memoria 
condivisa anche se queste non sono state completamente popolate dai processi 
figli.
*/

#ifndef __unix__

#error "Cannot compile this program on non-Unix systems"

#else
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define quit(msg) \
	do { printf("%s\n", msg); exit(EXIT_FAILURE); } while (0)

#define PAGE_SIZE 4096

#define CHILD 1
#define PARENT 0

int num_process;
struct sembuf sops;

char *shm_addr;
char **mem;

void post_sem (int semid, int num_sem) {

	sops.sem_num = num_sem;
	sops.sem_op = 1;
	sops.sem_flg = 0;

	if (semop(semid, &sops, 1) == -1)
		handle_error("semop");
}

void wait_sem (int semid, int num_sem) {

	sops.sem_num = num_sem;
	sops.sem_op = -1;
	sops.sem_flg = 0;

	if (semop(semid, &sops, 1) == -1)
		handle_error("semop");
}


void handler(int signo, siginfo_t *a, void *b){

	int i;

	printf("handler activated\n");

	for (i=1; i < num_process; i++){	
		shm_addr = mem[i];
		while(strcmp(shm_addr,"\0")!=0){
			printf("Il padre legge: %s",shm_addr);
			shm_addr += strlen(shm_addr)+1;
		}
	}

	exit(EXIT_SUCCESS);
}

void destroy_all(int semid, int shmid) {}

int main(int argc, char **argv) {
	
	int shm_id, sem_id;
	shm_addr = (char *)malloc(sizeof(char) * PAGE_SIZE); // indirizzo
	mem = (char **)malloc(sizeof(char *) * PAGE_SIZE); // array degli indirizzi

	pid_t pid;

	FILE *fd;

	key_t shm_key = 6868;
	key_t sem_key = 3434;

	struct sigaction act;
	sigset_t set;

	num_process = argc;

	if (argc < 2)
		quit("Usage: ./prog pathaname_1 ... [pathname_n]");

	/*	sem section	*/

	sem_id = semget(sem_key, 2, 0666 | IPC_CREAT);
	if (sem_id == -1)
	    handle_error("semget");
	
	if (semctl(sem_id, CHILD, SETVAL, num_process+1) == -1) //il figlio entra tante volte quanti sono i pathname
		handle_error("semctl");

	if (semctl(sem_id, PARENT, SETVAL, 0) == -1)
		handle_error("semctl");

	/*	main section */
	pid = fork();

	shm_id = shmget(shm_key, PAGE_SIZE, 0666 | IPC_CREAT);

	if (shm_id == -1)
	    handle_error("shmget");


	for (int i = 1; i < argc; i++) {
		mem[i] = shmat(shm_id, NULL, 0);
		if (mem[i] == (void *)-1)
		    handle_error("shmat");
		
		shm_addr = mem[i];

		fd = fopen(argv[i], "r");
        if (fd == NULL)
            quit("Cannot open file");

		if (pid == 0) {
			wait_sem(sem_id, CHILD);

            while (fscanf(fd, "%s\n", shm_addr)!= EOF) {
				printf("Lettura: %s\n", shm_addr);
				shm_addr += strlen(shm_addr)+1;
            }

            fclose(fd);
			post_sem(sem_id, PARENT);
		}
	}
	
	sigfillset(&set);

	act.sa_sigaction = handler;
	act.sa_mask = set;
	act.sa_flags  = 0;

	sigaction(SIGINT, &act,NULL);

	if (pid > 0) {
		wait_sem(sem_id, PARENT);
		while(1) pause();
		post_sem(sem_id, CHILD);
	}
}