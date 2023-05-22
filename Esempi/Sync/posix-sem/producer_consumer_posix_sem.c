#ifdef __unix__

#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/types.h>

#else
#error "Cannot compile on non-UNIX systems"

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE);}while(0)


#define mem_removal(shm_id, sem, addr) \
	{ shmctl(shm_id, IPC_RMID, 0); \
	  sem_destroy(&sem); \
	  shmdt(addr);}

#define x86_PAGE_SIZE 4096
#define OTHER_PAGE_SIZE 8196

#define BUFFER 100

void producer(sem_t sem, sem_t sem2,void* addr) {
	
	sem_wait(&sem); //Entra in discoteca
	sem_post(&sem2);
	printf("Scrvi qualcosa per il cosnumer: ");
	if(fgets((char *)addr, BUFFER, stdin) == NULL)
		handle_error("fgets");
	
	sem_post(&sem);
}

void consumer(sem_t sem, void *addr) {
	
	sem_wait(&sem);

//	printf("Il producer ha scritto: %s", addr);
	printf("Caio");
	sem_post(&sem);
}

int main() {

	sem_t mutex;
	sem_t mt;
	key_t shm_key = IPC_PRIVATE;
	
	int shm_id;
	void* shm_addr;
	
	pid_t pid;

#ifdef x86

	shm_id = shmget(shm_key, x86_PAGE_SIZE, IPC_CREAT | 0666);

#else
	shm_id = shmget(shm_key, OTHER_PAGE_SIZE, IPC_CREAT | 0666);

#endif
	if (shm_id == -1)
		handle_error("shmget");

	shm_addr = shmat(shm_id, NULL, 0);
	if (shm_addr == (void *) 1)
		handle_error("shmat");

	if (sem_init(&mutex, 1, 1) == -1)
		handle_error("sem_init");
	
	if (pid == 0) {
		producer(mutex, mt ,shm_addr);
	} else if (pid > 0) {
		consumer(mutex, shm_addr);
		
	} else {
		mem_removal(shm_id, mutex, shm_addr);
	}

}
