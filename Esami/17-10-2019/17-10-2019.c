/*
  SPECIFICATION TO BE IMPLEMENTED:
  Implementare una programma che riceva in input, tramite argv[], il nomi
  di N file (con N maggiore o uguale a 1).
  Per ogni nome di file F_i ricevuto input dovra' essere attivato un nuovo
  thread T_i. Il main thread dovra' leggere indefinitamente stringhe dallo
  standard-input e dovra' rendere ogni stringa letta disponibile ad uno solo
  degli altri N thread secondo uno schema circolare. Ciascun thread T_i a sua
  volta, per ogni stringa letta dal main thread e resa a lui disponibile, dovra'
  scriverla su una nuova linea del file F_i.

  L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
  WinAPI) in modo tale che quando il processo venga colpito esso dovra'
  riversare su standard-output e su un apposito file chiamato "output-file" il
  contenuto di tutti i file F_i gestiti dall'applicazione
  ricostruendo esattamente la stessa sequenza di stringhe (ciascuna riportata su
  una linea diversa) che era stata immessa tramite lo standard-input.

  In caso non vi sia immissione di dati sullo standard-input, l'applicazione
  dovra' utilizzare non piu' del 5% della capacita' di lavoro della CPU.


  NOTE:
    Compile using: make mutex_var for mutex implementation
                   make sem_var for semaphore implementation
*/

#ifdef __linux__

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <unistd.h>

#else
#error

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define error(msg)                                                             \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0);

#ifdef __mutex_var

pthread_mutex_t *read;
pthread_mutex_t *write;

#endif

#ifdef __sem_var

sem_t *read;
sem_t *write;

#endif

FILE *f;
char *buffer;
char **rec_buffers;

int num_td;

void handler(int dummy) {}

void *td_func(void *args) {}

int main(int argc, char **argv) {

  int i;

  if (argc < 2) {
    printf("Usage: ./prog F1 [F2] ... [Fn]");
    exit(-1);
  }

  num_td = argc - 1;
  pthread_t tid;

#ifdef __mutex_var

  read = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
  write = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));

  if (read == NULL || write == NULL)
    error("malloc");

  if (pthread_mutex_init(read, NULL) || pthread_mutex_init(write, NULL))
    error("pthread_mutex_init");

#endif

#ifdef __sem_var

  read = (sem_t *)malloc(sizeof(sem_t) * argc);
  write = (sem_t *)malloc(sizeof(sem_t) * argc);

  if (read == NULL || write == NULL)
    error("malloc");

  if (sem_init(read, 0, 1) || sem_init(write, 0, 0))
    error("sem_init");

#endif

  for (i = 0; i < num_td; i++) {
    if (pthread_create(&tid, NULL, td_func, (void *)i))
      error("pthread_create");
  }

  while (1) {
  }
}
