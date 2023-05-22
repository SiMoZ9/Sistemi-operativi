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

#define sem_error(sem_id, shm_id ,addr, sem_arg) \
	{	semctl(sem_id, -1, IPC_RMID, NULL); \
		shmctl(shm_id, IPC_RMID, NULL); \
		shmdt(addr);	}

#define remove_data(sem_id, shm_id ,addr, sem_arg) \
	{ 	semctl(sem_id, -1, IPC_RMID, sem_arg); \
		shmctl(shm_id, IPC_RMID, NULL); \
		shmdt(addr);	}

#define PAGE_SIZE 4096

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *__buf;
};

int consumer(void *addr, int ds){
	struct sembuf buf[1];

	buf[0].sem_num = 0;
	buf[0].sem_op = -1;
	buf[0].sem_flg = 0;

	/* WAIT sulla prima istanza di Semaforo */
	if (semop(ds, buf, 1) == -1)
	    handle_error("semop");

	printf("Il consumer legge: %s", (char *) addr);

	buf[0].sem_num = 1;
	buf[0].sem_op = 1;
	buf[0].sem_flg = 0;

	/* SIGNAL sulla seconda istanza di Semaforo */
	if (semop(ds, buf, 1) == -1)
		handle_error("semop");

	return 1;
}

int main(void)
{
	key_t shm_key;
	key_t sem_key;

    int shm_id;
	int sem_id;
	int cons;

	void *addr;

	shm_key = 100;
	sem_key = 200;

	union semun sem_arg;
	
	//Shm

	if ((shm_id = shmget(shm_key, PAGE_SIZE, IPC_CREAT | 0666)) == -1)
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

	sem_arg.val = 0; /* set 1st instance of sem to 0 */
	
	if (semctl(sem_id, 0, SETVAL, sem_arg) == -1) {
		sem_error(sem_id, shm_id, addr, sem_arg);
		handle_error("semctl");
	}

	sem_arg.val = 1; /* set 2nd instance of sem to 1 */

	if (semctl(sem_id, 1, SETVAL, sem_arg) == -1) {
		sem_error(sem_id, shm_id, addr, sem_arg);
		handle_error("semctl");
	}

	do {
		cons = consumer(addr, sem_id);
	}while(cons);

	remove_data(sem_id, shm_id, addr, sem_arg);

	exit(EXIT_SUCCESS);
}
