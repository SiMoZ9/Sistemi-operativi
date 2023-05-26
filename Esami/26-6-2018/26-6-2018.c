/*
SPECIFICATION TO BE IMPLEMENTED:
Implementare un programma che riceva in input tramite argv[1] un numero
intero N maggiore o uguale ad 1 (espresso come una stringa di cifre 
decimali), e generi N nuovi thread. Ciascuno di questi, a turno, dovra'
inserire in una propria lista basata su memoria dinamica un record
strutturato come segue:

typedef struct _data{
	int val;
	struct _data* next;
} data; 

I record vengono generati e popolati dal main thread, il quale rimane
in attesa indefinita di valori interi da standard input. Ad ogni nuovo
valore letto avverra' la generazione di un nuovo record, che verra'
inserito da uno degli N thread nella sua lista.

L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
WinAPI) in modo tale che quando il processo venga colpito esso dovra' 
stampare a terminale il contenuto corrente di tutte le liste (ovvero 
i valori interi presenti nei record correntemente registrati nelle liste
di tutti gli N thread).
*/

#ifndef __unix__

#error "Cannot compile this program on non-Unix systems"

#else
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0)


typedef struct _data {
	int val;
	struct list* next;
}data;

pthread_mutex_t lock;
pthread_mutex_t next;
data *lists;
int num_threads;
int val;

void *thread_func(void *arg) {
	data *l;

	printf("Thread %lu started\n", (long)arg);

	while (1) {
		l = (data *)malloc(sizeof(data) * num_threads);

		if (pthread_mutex_lock(&lock) != 0)
		    handle_error("pthread_mutex_lock");

		printf("thread %ld - found value is %d\n",(long)arg, val);
		l->val = val;
		if (pthread_mutex_unlock(&next)!= 0)
		    handle_error("pthread_mutex_unlock");
		
		l -> next = lists[(int)arg].next;
		lists[(int)arg].next = l;
	}
}

int main(int argc, char **argv) {
	long i;
	pthread_t td;
	
	sigset_t set;
	struct sigaction act;

	if (argc != 2) {
		printf("Usage: ./prog <n_threads>");
		exit(EXIT_FAILURE);
	}

	num_threads = strtol(argv[1], NULL, 10);

	if (num_threads < 1) {
		printf("Number of threads must be geq than 1");
		exit(EXIT_FAILURE);
	}

	lists = (data *)malloc(sizeof(data) * num_threads);
	if (lists == NULL)
		handle_error("malloc");

	/*	init list	*/

	for (i = 0; i < num_threads; i++) {
		lists[i].val = -1;
        lists[i].next = NULL;
	}

	if (pthread_mutex_init(&lock, NULL) || pthread_mutex_init(&next, NULL))
		handle_error("pthread_mutex_init");

	for (i = 0; i < num_threads; i++) {
        if (pthread_create(&td, NULL, (void*)thread_func, (void*)i))
            handle_error("pthread_create");
    }

	while (1) {
		pthread_mutex_lock(&next);
		scanf("%d",&val);
		pthread_mutex_unlock(&lock);
	}
}