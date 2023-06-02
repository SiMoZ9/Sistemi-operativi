/*
  SPECIFICATION TO BE IMPLEMENTED:
  Implementare un programma che riceva in input tramite argv[] i pathname
  associati ad N file (F1 ... FN), con N maggiore o uguale ad 1.
  Per ognuno di questi file generi un thread che gestira' il contenuto del file.
  Dopo aver creato gli N file ed i rispettivi N thread, il main thread dovra'
  leggere indefinitamente la sequenza di byte provenienti dallo standard-input.
  Ogni 5 nuovi byte letti, questi dovranno essere scritti da uno degli N thread
  nel rispettivo file. La consegna dei 5 byte da parte del main thread
  dovra' avvenire secondo uno schema round-robin, per cui i primi 5 byte
  dovranno essere consegnati al thread in carico di gestire F1, i secondi 5
  byte al thread in carico di gestire il F2 e cosi' via secondo uno schema
  circolare.

  L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
  WinAPI) in modo tale che quando il processo venga colpito esso dovra',
  a partire dai dati correntemente memorizzati nei file F1 ... FN, ripresentare
  sullo standard-output la medesima sequenza di byte di input originariamente
  letta dal main thread dallo standard-input.

  Qualora non vi sia immissione di input, l'applicazione dovra' utilizzare
  non piu' del 5% della capacita' di lavoro della CPU.
*/

#ifndef __linux__

#error "Cannot compile on non-Linux systems"

#else

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/sem.h>
#include <unistd.h>

#endif

#include <stdio.h>
#include <stdlib.h>

#define error(msg)                                                             \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0);

pthread_mutex_t mutex;
pthread_mutex_t lock;

char buffer[4096];
int fd;
ssize_t wd;

struct sembuf sops;

void *pass(void *args) {

  long me = (int)args - 1;

  pthread_mutex_lock(&lock);

  printf("Exec thread %d", me);

  while (1) {

    wd = write(fd, buffer, 5);

    if (wd == -1)
      error("write");

    if (wd > 5) {
      printf("Cannot write more than 5 bytes");
      exit(EXIT_FAILURE);
    }

    pthread_mutex_unlock(&mutex);
  }
}

int main(int argc, char **argv) {

  sigset_t sig;
  long i;
  pthread_t tid;

  if (argc < 2) {
    printf("Usage: ./prog F1 [F2] ... [F2]");
    exit(EXIT_FAILURE);
  }

  pthread_mutex_init(&mutex, NULL);
  pthread_mutex_init(&lock, NULL);

  for (i = 1; i < argc; i++) {
    if (pthread_create(&tid, NULL, pass, (void *)i) == -1)
      error("pthread_create");

    fd = open(argv[i], O_CREAT | O_TRUNC | O_RDWR, 0666);
    if (fd == -1)
      error("open");
  }

  while (1) {
    pthread_mutex_lock(&mutex);
    scanf("%ms", buffer);
    pthread_mutex_unlock(&lock);
  }
}
