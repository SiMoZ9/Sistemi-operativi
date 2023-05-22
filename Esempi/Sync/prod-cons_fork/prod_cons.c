#ifdef __unix__

#include <unistd.h>
#include <signal.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/types.h>

#else
#error "Cannot compile on non-Unix systems"

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

char buffer[4096];

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *__buf;
};

int consumer(void *addr, int ds){
	struct sembuf buf[1];
	char *buffer = (char *)addr;

	buf[0].sem_num = 0;
	buf[0].sem_op = -1;
	buf[0].sem_flg = 0;

	/* WAIT sulla prima istanza di Semaforo */
	if (semop(ds, buf, 1) == -1)
	    handle_error("semop");

	printf("Il consumer legge: %s", buffer);


	buf[0].sem_num = 1;
	buf[0].sem_op = 1;
	buf[0].sem_flg = 0;

	/* SIGNAL sulla seconda istanza di Semaforo */
	if (semop(ds, buf, 1) == -1)
		handle_error("semop");

	return 1;
}

int producer(void *addr, int ds){
	struct sembuf buf[1]; //2 semaphores instance
	
	buf[0].sem_num = 1;
	buf[0].sem_op = -1; //wait
	buf[0].sem_flg = 0;

	if(semop(ds, buf, 1) == -1)
		handle_error("semop");
	
	do {
		printf("Scrivi qualcosa per il consumatore [quit per uscire]: ");	
		if(fgets((char *)addr, PAGE_SIZE, stdin) == NULL)
	   		 handle_error("fgets");
		printf("%s", addr);
		if(strcmp(addr, "quit\n"));
			break;
	}while(1);

	buf[0].sem_num = 0;
	buf[0].sem_op = 1;
	buf[0].sem_flg = 0;

	if(semop(ds, buf, 1) == -1)
		handle_error("semop");

	return 1;
}

int main() {
	key_t shm_key;
	key_t sem_key;
	
	pid_t pid;

	int shm_id;
	int sem_id;
	int prod, cons;

	void *addr;

	shm_key = 100;
	sem_key = 200;
	
	union semun un;

	
	/* SECTION 
	 * before fork
	 * */

	if ((shm_id = shmget(shm_key, PAGE_SIZE, IPC_CREAT | 0666)) == -1)
		handle_error("shmget");

	if ((addr = shmat(shm_id, NULL, 0)) == (void *) -1) {
		shmctl(shm_id, IPC_RMID, NULL);
		handle_error("shmat");
	}

	if ((sem_id = semget(sem_key, 2, IPC_CREAT | 0666)) == -1) {
		shmctl(shm_id, IPC_RMID, NULL);
		shmdt(addr);
		handle_error("semget");
	}
	
	un.val = 0;

	if (semctl(sem_id, 0, SETVAL, un)) {
		sem_error(sem_id, shm_id, addr, un);
		handle_error("semctl");
	}
	
	un.val = 1;

	if (semctl(sem_id, 1, SETVAL, un)) {
		sem_error(sem_id, shm_id, addr, un);
		handle_error("semctl");
	}
	/*END SECTION
	 * before fork
	 * */
	
	pid = fork();

	if (pid == 0) {
		cons = consumer(addr, sem_id);;
	} else if (pid > 0) {
		producer(addr, sem_id);
	}

	remove_data(sem_id, shm_id, addr, un);
}
