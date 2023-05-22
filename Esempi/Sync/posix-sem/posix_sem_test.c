#define _GNU_SOURCE

#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

sem_t mutex;

void* routine() {
	sem_wait(&mutex); //prende il gettone e entra in discoteca
	printf("Sono il thread (%lu) e sono entrato in sezione critica\n", pthread_self());
	sleep(1);
	printf("Finito\n");
	sem_post(&mutex);
}

int main(int argc, char **argv) {
	
	int sem;
	pthread_t t1, t2;
	
	if ((sem = sem_init(&mutex, 0, 1)) == -1) {
		perror("sem_init");
		exit(1);
	}

	if (pthread_create(&t1, NULL, routine, NULL) != 0) {
		perror("pthread_create");
		exit(1);
	}

	if (pthread_create(&t2, NULL, routine, NULL) != 0) {
		perror("pthread_create");
		exit(1);
	}

	//join
	
	if (pthread_join(t1, NULL) != 0) {
		perror("pthread_join");
		exit(1);
	}

	if (pthread_join(t2, NULL) != 0) {
		perror("pthread_join");
		exit(1);
	}
}


