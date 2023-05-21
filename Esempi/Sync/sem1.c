#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/types.h>

#define handle_error(x) \
	do{perror(x); exit(EXIT_FAILURE);}while(0)

#define PAGE_SIZE 4096

#define BUFF_SIZE 100

char buff[100];

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *__buf;
};

int producer(void *addr, int ds){
	struct sembuf buf[1]; //2 semaphores instance
	
	buf[0].sem_num = 1;
	buf[0].sem_op = -1; //wait
	buf[0].sem_flg = 0;

	if(semop(ds, buf, 1) == -1)
		handle_error("semop");
	
	printf("Scrivi qualcosa per il consumatore: ");
	if(fgets(buff, 100, stdin) == NULL)
		handle_error("fgets");
	
	buf[0].sem_num = 0;
	buf[0].sem_op = 1;
	buf[0].sem_flg = 0;

	if(semop(ds, buf, 1) == -1)
		handle_error("semop");

	return 1;
}

int consumer(void *addr, int ds){
	struct sembuf buf[1]; //2 semaphores instance
	
	buf[0].sem_num = 0;
	buf[0].sem_op = -1; //wait
	buf[0].sem_flg = 0;

	if(semop(ds, buf, 1) == -1)
		handle_error("semop");
	
	printf("Il consumatore legge: %s", buff);

	buf[0].sem_num = 1;
	buf[0].sem_op = 1;
	buf[0].sem_flg = 0;

	if(semop(ds, buf, 1) == -1)
		handle_error("semop");
	return 1;
}

int main(void)
{
	key_t sem_key = IPC_PRIVATE;
	key_t shm_key = IPC_PRIVATE;
	int semid = semget(sem_key, 2, IPC_CREAT | 0666);
	int shmid = shmget(shm_key, PAGE_SIZE, IPC_CREAT | 0666);
	void *shm_addr;

	union semun sem_arg;

	int cons, prod;

	/* shared memory error check */
	if(shmid == -1)
		handle_error("shmget");

	if(shm_addr = shmat(shmid, NULL, 0) == (void *) -1)
		handle_error("shmat");
	/* end shm error check */

	/* semaphore error check */
	if(semid == -1)
		handle_error("semget");
	
	sem_arg.val = 0;

	if(semctl(semid, 0, SETVAL, sem_arg) == -1)
	{
		semctl(semid, -1, IPC_RMID, sem_arg);
		shmctl(shmid, IPC_RMID, NULL);
		shmdt(shm_addr);
		handle_error("semctl");
	}

	sem_arg.val = 1;
	
	if(semctl(semid, 1, SETVAL, sem_arg) == -1)
	{
		semctl(semid, -1, IPC_RMID, sem_arg);
		shmctl(shmid, IPC_RMID, NULL);
		shmdt(shm_addr);
		handle_error("semctl");
	}
	
	do{
		prod = producer(shm_addr, semid);
		cons = consumer(shm_addr, semid);
	}while(1);

	/* removing shm and sem */

	semctl(semid, 0, IPC_RMID, 0);
	shmctl(shmid, IPC_RMID, NULL);
	shmdt(shm_addr);

	/* end */

	exit(EXIT_SUCCESS);
}
