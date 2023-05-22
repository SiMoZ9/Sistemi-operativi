
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <signal.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/types.h>

#define handle_error(x) \
	do{perror(x); exit(EXIT_FAILURE);}while(0)

#define PAGE_SIZE 4096

int producer(void *addr, int ds){
	struct sembuf buf[1]; //2 semaphores instance
	
	buf[0].sem_num = 1;
	buf[0].sem_op = -1; //wait
	buf[0].sem_flg = 0;

	if(semop(ds, buf, 1) == -1)
		handle_error("semop");
	
	printf("Scrivi qualcosa per il consumatore: ");

	if(fgets(addr, PAGE_SIZE, stdin) == NULL)
	    handle_error("fgets");

	buf[0].sem_num = 0;
	buf[0].sem_op = 1;
	buf[0].sem_flg = 0;

	if(semop(ds, buf, 1) == -1)
		handle_error("semop");

	return 1;
}

int main(void)
{
	key_t shm_key;
	key_t sem_key;

    int shm_id;
	int sem_id;
	int prod;

	void *addr;

	shm_key = 100;
	sem_key = 200;

	if ((shm_id = shmget(shm_key, PAGE_SIZE, 0)) == -1)
	    handle_error("shmget");

	if ((addr = shmat(shm_id, NULL, 0)) == (void *) -1) {
		shmctl(shm_id, IPC_RMID, NULL);
		handle_error("shmat");
	}
	//Sem

	if ((sem_id = semget(sem_key, 2, IPC_CREAT | 0666)) == -1) {
		shmctl(shm_id, IPC_RMID, NULL);
		shmdt(addr);
		handle_error("semget");
	}
        
	do {
		prod = producer(addr, sem_id);
	}while(prod);

	shmdt(addr);

	/* end */

	exit(EXIT_SUCCESS);
}
