#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/shm.h>

#define PAGE_SIZE 4096

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); }while(0);

#define READER 0
#define WRITER 1

int main(void) {
	int shmid;
	
	int semid;

	key_t shm_key = 6868;
	key_t sem_key = 3434;

	pid_t pid = fork();

	char *addr;
	
	char *buffer[100];

	struct sembuf sem_op;
		
	/*	shared memory init	*/

	shmid = shmget(shm_key, PAGE_SIZE, 0666 | IPC_CREAT);

	if (shmid == -1)
		handle_error("shmget");

	addr = shmat(shmid, NULL, 0);


	if (addr == (void *) -1)
		handle_error("shmat");

	/*	sem init	*/

	semid = semget(sem_key, 2, IPC_CREAT | 0666);

	if (semid == -1)
		handle_error("semget");

	semctl(semid, READER, SETVAL, 0);
	semctl(semid, WRITER, SETVAL, 1);

	if (pid == 0) {
		do {
		sem_op.sem_num = READER;
            	sem_op.sem_op = -1;
            	sem_op.sem_flg = 0;
           	semop(semid, &sem_op, 1);
		
		
		printf("Il figlio legge: %s", addr);

		sem_op.sem_num = WRITER;
            	sem_op.sem_op = 1;
            	sem_op.sem_flg = 0;
            	semop(semid, &sem_op, 1);
		
		sleep(1);
		} while(1);

	} else if (pid > 0) {
		
		do {
            	sem_op.sem_num = WRITER;
            	sem_op.sem_op = -1;
            	sem_op.sem_flg = 0;
            	semop(semid, &sem_op, 1);
		
		int i = 0;
			
	    	printf("Scrivi qualcosa per il figlio: ");
		fgets(addr, PAGE_SIZE, stdin);

            	sem_op.sem_num = READER;
            	sem_op.sem_op = 1;
            	sem_op.sem_flg = 0;
            	semop(semid, &sem_op, 1);

		sleep(1);
		} while(1);

	} else {
		handle_error("fork");
	}

	shmctl(shmid, IPC_RMID, NULL);
	semctl(semid, WRITER, IPC_RMID);
	semctl(semid, READER, IPC_RMID);
}
