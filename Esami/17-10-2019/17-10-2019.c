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

pthread_mutex_t *reader;
pthread_mutex_t *writer;

#endif

#ifdef __sem_var

sem_t *reader;
sem_t *writer;

#endif

char **buffer;

FILE *output;
FILE **files;

char **argv_names;

int num_td;

void handler(int dummy) {}

void *td_func(void *args) {

  long me = (long)args;
  printf("thread %lu is setting up\n", me);
  fflush(stdout);

  FILE *fp;
  fp = fopen(argv_names[me], "w+");

  if (fp == NULL)
    error("fopen");

  while (1) {
#ifdef __mutex_var
    if (pthread_mutex_lock(reader + me))
      error("pthread_mutex_lock");

    printf("thread %lu is writing its string on file %s\n", me, buffer[me]);
    fprintf(output, "%s\n", buffer[me]);
    fflush(output);

    if (pthread_mutex_unlock(writer + me))
      error("pthread_mutex_lock");
#endif

#ifdef __sem_var

    sem_wait(reader + me);

    printf("thread %lu is writing its string on file %s\n", me, buffer[me]);
    fprintf(output, "%s\n", buffer[me]);
    fflush(output);

    sem_post(writer + me);
#endif
  }
}

int main(int argc, char **argv) {

  int i;
  int ret;

  char *p;

  if (argc < 2) {
    printf("Usage: ./prog F1 [F2] ... [Fn]");
    exit(-1);
  }

  num_td = argc - 1;
  pthread_t tid;

  buffer = (char **)malloc(4096);
  argv_names = (char **)malloc(sizeof(argv));
  files = (FILE **)malloc(sizeof(FILE *) * num_td);

  argv_names = argv + 1;

  output = fopen("output.txt", "w+");
  if (output == NULL)
    error("fopen");

#ifdef __mutex_var

  reader = malloc(sizeof(pthread_mutex_t) * argc);
  writer = malloc(sizeof(pthread_mutex_t) * argc);

  if (reader == NULL || writer == NULL)
    error("malloc");

  for (i = 0; i < num_td; i++) {
    if (pthread_mutex_init(reader + i, NULL) ||
        pthread_mutex_init(writer + i, NULL) || pthread_mutex_lock(reader + i))
      error("pthread_mutex_init");
  }

#endif

#ifdef __sem_var

  reader = (sem_t *)malloc(sizeof(sem_t) * argc);
  writer = (sem_t *)malloc(sizeof(sem_t) * argc);

  if (reader == NULL || writer == NULL)
    error("malloc");

  for (i = 0; i < num_td; i++) {

    if (sem_init(reader + i, 0, 1) || sem_init(writer + i, 0, 0))
      error("sem_init");

    sem_wait(reader + i);
  }

#endif

  for (i = 0; i < num_td; i++) {
    if (pthread_create(&tid, NULL, td_func, (void *)i))
      error("pthread_create");
  }

  int turn = 0;
  while (1) {
    ret = scanf("%ms", &p);

#ifdef __mutex_var
    pthread_mutex_lock(writer + turn);
#endif

#ifdef __sem_var
    sem_wait(writer + turn);
#endif

    if (ret == -1 || errno == EINTR)
      error("scanf");

    buffer[turn] = p;

#ifdef __sem_var
    sem_post(reader + turn);
#endif

#ifdef __mutex_var
    pthread_mutex_unlock(reader + turn);
#endif

    turn = (turn + 1) % (num_td);
  }
}
