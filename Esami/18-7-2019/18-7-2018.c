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

int main(int argc, char **argv) {
	
	int shm_id, sem_id;
	char *shm_addr;
	void **mem = malloc(sizeof(void *) * PAGE_SIZE);

	pid_t pid;

	FILE *fd;

	key_t shm_key = IPC_PRIVATE;
	key_t sem_key = IPC_PRIVATE;

	struct sembuf sops;
	
	num_process = argc;

	if (argc < 2)
		quit("Usage: ./prog <pathaname_1> ... <pathname_n>");

	/*	sem section	*/

	sem_id = semget(sem_key, 2, 0666 | IPC_CREAT);
	if (sem_id == -1)
	    handle_error("semget");
	
	if (semctl(sem_id, CHILD, SETVAL, 1) == -1)
		handle_error("semctl");

	if (semctl(sem_id, PARENT, SETVAL, 0) == -1)
		handle_error("semctl");

	/*	main section */
	pid = fork();

	for (int i = 1; i < num_process; i++) {
		mem[i] = mmap(NULL,PAGE_SIZE, PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_SHARED,0,0);
		if (mem[i] == NULL)
		    handle_error("mmap");

		shm_addr = mem[i];

		if (pid < 0) {
			handle_error("fork");
		} else if (pid == 0) {
			fd = fopen(argv[i], "r");
			sops.sem_num = CHILD;
			sops.sem_op = -1;
			sops.sem_flg = 0;

			if (semop(sem_id, &sops, 1) == -1)
			    handle_error("semop");

			printf("Il figlio scrive in memoria condivisa\n");

			while(fscanf(fd, "%s\n", shm_addr) != EOF) {
			    printf("Letto: %s\n", shm_addr);
				shm_addr += strlen(shm_addr)+1;
			}

			sops.sem_num = PARENT;
			sops.sem_op = 1;
			sops.sem_flg = 0;

			if (semop(sem_id, &sops, 1) == -1)
			    handle_error("semop"); 

		}
	}

	if (pid > 0) {
			sops.sem_num = PARENT;
			sops.sem_op = -1;
			sops.sem_flg = 0;

			if (semop(sem_id, &sops, 1) == -1)
			    handle_error("semop");

			printf("Il padre legge dalla memoria condivisa: %s\n", shm_addr);
			for (int i=1; i < num_process; i++){	
				shm_addr = mem[i];
				while(strcmp(shm_addr,"\0")!=0){
					printf("%s\n",shm_addr);
					shm_addr += strlen(shm_addr)+1;
				}
			}
			sops.sem_num = CHILD;
			sops.sem_op = 1;
			sops.sem_flg = 0;

			if (semop(sem_id, &sops, 1) == -1)
			    handle_error("semop");
		}
}