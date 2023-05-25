#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>

#define PAGE_SIZE 4096

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); }while(0);

#define READER 0
#define WRITER 1

int main(void) {
	
	int semid;

	key_t sem_key = 3434;

	pid_t pid = fork();

	
	char buffer[100];
	char *str_arr[100];

	struct sembuf sem_op;
	
	int i = 0;

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
		
		
		printf("Il figlio legge: %s", *buffer);

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
			
		do {
	    		printf("Scrivi qualcosa per il figlio: ");
			fgets(buffer, PAGE_SIZE, stdin);
			
			str_arr[i] = buffer;
			i++;
		} while(strcmp(buffer, "quit\n"));

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
