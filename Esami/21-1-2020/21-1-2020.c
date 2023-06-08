
/*
  SPECIFICATION TO BE IMPLEMENTED:
  Implementare una programma che ricevento in input tramite argv[] una stringa S
  esegua le seguenti attivita'.
  Il main thread dovra' attivare due nuovi thread, che indichiamo con T1 e T2.
  Successivamente il main thread dovra' leggere indefinitamente caratteri dallo
  standard input, a blocchi di 5 per volta, e dovra' rendere disponibili i byte
  letti a T1 e T2.



  Il thread T1 dovra' inserire di volta in volta i byte ricevuti dal main thread
  in coda ad un file di nome S_diretto, che dovra' essere creato.



  Il thread T2 dovra' inserirli invece nel file S_inverso, che dovra' anche esso
  essere creato, scrivendoli ogni volta come byte iniziali del file (ovvero in
  testa al file secondo uno schema a pila).

  L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
  WinAPI) in modo tale che quando il processo venga colpito esso dovra'
  calcolare il numero dei byte che nei due file hanno la stessa posizione ma
  sono tra loro diversi in termini di valore. Questa attivita' dovra' essere
  svolta attivando per ogni ricezione di segnale un apposito thread.

  In caso non vi sia immissione di dati sullo standard input, l'applicazione
  dovra' utilizzare non piu' del 5% della capacita' di lavoro della CPU.
*/

#ifdef __linux__

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#else

#error

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define error(msg, stream)                                                     \
  do {                                                                         \
    perror(msg);                                                               \
    fflush(stream);                                                            \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

#define STRING_SIZE 4096

int S_diretto;
int S_inverso;

char *buffer;

char **rdbf1;
char **rdbf2;

char **args;

pthread_mutex_t *ready;
pthread_mutex_t *done;

void handler(int dummy) {

  int ret1, ret2;

  rdbf1 = (char **)malloc(sizeof(char *) * STRING_SIZE);
  rdbf2 = (char **)malloc(sizeof(char *) * STRING_SIZE);

  ret1 = read(S_diretto, rdbf1, sizeof(rdbf1));
  ret2 = read(S_inverso, rdbf2, sizeof(rdbf2));

  if (ret1 < 0 || ret2 < 0)
    error("read", stdout);
}

void *thread1_function(void *args) {

  printf("thread 1 setup\n");
  fflush(stdout);

  while (1) {

    pthread_mutex_lock(ready);

    printf("thread 1 is writing in the bottom of the file\n");
    fflush(stdout);

    if (write(S_diretto, buffer, 5) == -1)
      error("write", stdout);

    pthread_mutex_unlock(done);
  }
}

void *thread2_function(void *args) {

  printf("thread 2 setup\n");
  fflush(stdout);

  while (1) {
    pthread_mutex_lock(ready);

    lseek(S_inverso, 0, SEEK_SET);

    printf("thread 2 is writing in the top of the file\n");
    fflush(stdout);
    if (write(S_inverso, buffer, 5) == -1)
      error("write", stdout);

    pthread_mutex_unlock(done);
  }
}

int main(int argc, char **argv) {

  pthread_t tid1, tid2;
  int ret;
  int i;

  sigset_t set;
  struct sigaction act;

  if (argc != 2) {
    printf("usage: ./prog S");
    exit(EXIT_FAILURE);
  }

  ready = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * 3);
  done = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t) * 3);

  buffer = (char *)malloc(STRING_SIZE);
  args = (char **)malloc(sizeof(char *) * argc);

  args = argv - 1;

  S_diretto = open("S_diretto", O_CREAT | O_TRUNC | O_RDWR, 0664);
  S_inverso = open("S_inverso", O_CREAT | O_TRUNC | O_RDWR, 0664);

  if (pthread_mutex_init(ready, NULL) || pthread_mutex_init(done, NULL))
    error("pthread_mutex_init", stdout);

  if (pthread_create(&tid1, NULL, thread1_function, NULL) ||
      pthread_create(&tid2, NULL, thread2_function, NULL))
    error("pthread_create", stdout);

  pthread_mutex_lock(ready);

  while (1) {

    pthread_mutex_lock(done);
    for (i = 0; i < 5; i++)
      buffer[i] = getchar();

    if (errno == EINTR)
      error("", stdout);

    pthread_mutex_unlock(ready);
  }
}
