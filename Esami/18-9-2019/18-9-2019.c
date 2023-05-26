/*SPECIFICATION TO BE IMPLEMENTED:
	Implementare una programma che riceva in input, tramite argv[], un insieme di
	stringhe S_1 ..... S_n con n maggiore o uguale ad 1. 
	Per ogni stringa S_i dovra' essere attivato un thread T_i.
	Il main thread dovra' leggere indefinitamente stringhe dallo standard-input.
	Ogni stringa letta dovra' essere resa disponibile al thread T_1 che dovra' 
	eliminare dalla stringa ogni carattere presente in S_1, sostituendolo con il 
	carattere 'spazio'.
	Successivamente T_1 rendera' la stringa modificata disponibile a T_2 che dovra' 
	eseguire la stessa operazione considerando i caratteri in S_2, e poi la passera' 
	a T_3 (che fara' la stessa operazione considerando i caratteri in S_3) e cosi' 
	via fino a T_n. 
	T_n, una volta completata la sua operazione sulla stringa ricevuta da T_n-1, dovra'
	passare la stringa ad un ulteriore thread che chiameremo OUTPUT il quale dovra' 
	stampare la stringa ricevuta su un file di output dal nome output.txt.
	Si noti che i thread lavorano secondo uno schema pipeline, sono ammesse quindi 
	operazioni concorrenti su differenti stringhe lette dal main thread dallo 
	standard-input.

	L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
	WinAPI) in modo tale che quando il processo venga colpito esso dovra' 
	stampare il contenuto corrente del file output.txt su standard-output.

	In caso non vi sia immissione di dati sullo standard-input, l'applicazione 
	dovra' utilizzare non piu' del 5% della capacita' di lavoro della CPU.
*/

#ifdef __unix__
#define _GNU_SOURCE

#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/ipc.h>

#else
#error "Cannot compile on non-UNIX systems"

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); }while(0)

#define PAGE_SIZE 4096

pthread_mutex_t *ready;
pthread_mutex_t *done;

FILE *fd;

int thread_counter;

char **strings;
char **buffers;
char *in_str;

void out(int dummy){
	system("cat output.txt");
	exit(0);
}

void *str_to_space(void *args){

	long int me = (long int)args;
	int i,j;

	if(me < thread_counter -1){
		printf("thread %ld legge la stringa %s\n",me,strings[me]);
	}
	else{
		printf("thread %ld deve scrivere\n",me);
	}
	fflush(stdout);

	while(1){
		if(pthread_mutex_lock(ready+me)){
			printf("mutex lock attempt error\n");
			exit(EXIT_FAILURE);
		}

		printf("thread %ld - got string %s\n",me,buffers[me]);

		if(me == thread_counter-1){
			printf("Il thread sta scrivendo nel file la stringa %s\n",buffers[me]);
			fprintf(fd,"%s\n",buffers[me]);//each string goes to a new line
			fflush(fd);
		}
		else{
			for(i=0; i<strlen(strings[me]);i++){
				for(j=0;j<strlen(buffers[me]);j++){
					if(*(buffers[me]+j) == *(strings[me]+i)) *(buffers[me]+j) = ' ';
				}

			}	

			pthread_mutex_lock(done+me+1);
			buffers[me+1] = buffers[me];
			pthread_mutex_unlock(ready+me+1);
		}
	
		pthread_mutex_unlock(done+me);
	}
}


int main (int argc, char **argv) {

	long int i;
	int ret;
	char *p;

	if (argc < 2) {
		printf("Usage: ./prova.o <S1> <S2> ... <Sn>");
		exit(EXIT_FAILURE);
	}

	strings = argv + 1;
	thread_counter = argc;
	fd = fopen("output.txt", "w+");

	pthread_t td;

	ready = malloc(argc * sizeof(pthread_mutex_t));
	done = malloc(argc * sizeof(pthread_mutex_t));

	buffers = (char **)malloc(thread_counter * sizeof(char *));
	in_str = (char *)malloc(thread_counter * sizeof(char));

	for (i=0; i < thread_counter;i++){
			if (pthread_mutex_init(ready+i,NULL) ||pthread_mutex_init(done+i,NULL) || pthread_mutex_lock(ready+i)){
				handle_error("error");
			}
	}

	for (i=0; i<thread_counter;i++){
		ret = pthread_create(&td,NULL,str_to_space,(void*)i);
		if(ret != 0){
			printf("thread spawn failure\n");
			exit(EXIT_FAILURE);
		}

	}

	signal(SIGINT,out);

	while(1){
        ret = scanf("%ms",&p);

		pthread_mutex_lock(done);
		buffers[0] = p;
		
        pthread_mutex_unlock(ready);
	}
	pthread_mutex_destroy(ready); pthread_mutex_destroy(done);

	exit(0);
}
