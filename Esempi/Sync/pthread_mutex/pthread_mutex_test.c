#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 100

pthread_mutex_t mutex;

char buffer[BUFFER_SIZE];
int buffer_w = 0;

void *consumer(void *args)
{
	pthread_mutex_lock(&mutex);

	if (buffer_w == 1) {
		printf("Ho letto: %s", buffer);
		buffer_w = 0;
	}
	pthread_mutex_unlock(&mutex);

	return NULL;
}

void *producer(void *args)
{
	pthread_mutex_lock(&mutex);

	printf("Scrivi qualcosa: ");
	fgets(buffer, sizeof(BUFFER_SIZE), stdin);
	
	buffer_w = 1;

	pthread_mutex_unlock(&mutex);

	return NULL;
}

int main(void) {

	pthread_t prod, cons;
	pthread_mutex_init(&mutex, NULL);

	pthread_create(&cons, NULL, producer, NULL);
	pthread_create(&prod, NULL, consumer, NULL);

	pthread_join(prod, NULL);
	pthread_join(cons, NULL);

//	pthread_mutex_destroy(&mutex);
}
