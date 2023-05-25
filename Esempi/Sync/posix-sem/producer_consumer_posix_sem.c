#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/shm.h>

#define PAGE_SIZE 4096

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); }while(0);

int main(void) {
	int shmid;
	sem_t *reader;
	sem_t *writer;

	key_t key = 6868;

	pid_t pid = fork();

	void *addr;
		
	/*	shared memory init	*/

	shmid = shmget(key, PAGE_SIZE, 0666 | IPC_CREAT);

	reader = (sem_t *)mmap(NULL, sizeof(sem_t), PROT_READ, MAP_SHARED, 0, 0);

	printf("%d", reader);

//	sem_init(reader, 1, 0);
	//sem_init(writer, 1, 1);




	if (shmid == -1)
		handle_error("shmget");

	addr = shmat(shmid, NULL, 0);

	if (addr == (void *) -1)
		handle_error("shmat");


	if (pid == 0) {
		sem_wait(reader);

		printf("Il figlio legge: %s", (char *)addr);

		sem_post(writer);

		sleep(1);

	} else if (pid > 0) {

		sem_wait(writer);

		printf("Scrivi qualcosa per il figlio: ");
		fgets((char *)writer, PAGE_SIZE, stdin);

		printf("Hai scritto %s", (char *)addr);

		sem_post(reader);

		sleep(1);

	} else {
		handle_error("fork");
	}

	shmdt(reader);
	shmctl(shmid, IPC_RMID, NULL);

	sem_close(writer); sem_close(reader);

}
