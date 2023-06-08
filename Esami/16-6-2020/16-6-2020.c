/*
  Si scriva il codice di una funzione C (Posix oppure WinAPI a scelta dello
  studente) con la seguente interfaccia "void tunnel(int/handle descriptors[],
  int count)" tale che, se eseguita, porti l'applicazione a gestire, per ogni
  file-descriptor (o hanlde) dell'array "descriptors[]" l'inoltro del flusso dei
  dati in ingresso sul quel canale verso il canale standard-output
  dell'applicazione. Il parametro "count" indica di quanti elementi e'
  costituito l'array "descriptors[]". Tale inoltro dovra' essere attuato in modo
  concorrente per i diversi canali.
  La funzione "tunnel" dovra' anche attivare un gestore del segnale Posix SIGINT
  (o dell'evento CTRL_C_EVENT nel caso WinAPI) che dovra', se eseguito chiudere
  tutti i canali associati gli elementi dell'array "descriptors[]" portando al
  completamento delle operazioni di inoltro dei dati. E' compito dello studente
  scrivere anche il codice C del gestore.
*/

#ifdef __linux__

#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#else
#error

#endif

#include <stdio.h>
#include <stdlib.h>

#define error(msg)                                                             \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

int *descriptors;
int count;

int ret_r;

struct stat st;
char *bf;
sem_t *sem_a;
sem_t *sem_b;
sem_t *mutex;

void tunnel(int *fds, int count) {
  int i = 0;
  int ret_r;
  size_t size;

  for (i = 0; i < count; i++) {

    fstat(fds[i], &st);
    size = st.st_size;

    while (size != 0) {

      sem_wait(mutex);
      ret_r = read(fds[i], bf, size);
      if (ret_r == -1)
        error("read");

      size -= ret_r;

      sem_post(mutex);
    }
    printf("String %d %s\n", i, bf);
  }

  exit(EXIT_SUCCESS);
}

void handler(int dummy) {
  int i = 0;

  for (i = 0; i < count; i++) {
    sem_wait(mutex);
    close(descriptors[i]);
    sem_post(mutex);
  }
  printf("All descriptors are closed");

  kill(getppid(), SIGKILL);
}

int main(int argc, char **argv) {

  int i;
  pid_t pid;
  sigset_t set;

  struct sigaction act;

  if (argc < 2) {
    printf("usage: ./prog f1 [f2] ... [fn]");
    exit(EXIT_FAILURE);
  }

  count = argc - 1;

  sem_a = (sem_t *)malloc(sizeof(sem_t) * count);
  sem_b = (sem_t *)malloc(sizeof(sem_t) * count);
  mutex = (sem_t *)malloc(sizeof(sem_t) * 2);

  bf = (char *)malloc(100);

  sigemptyset(&set);
  sigfillset(&set);
  sigaddset(&set, SIGINT);
  sigprocmask(SIG_UNBLOCK, &set, NULL);

  if (sem_init(sem_a, 0, 1) || sem_init(sem_b, 0, 0) || sem_init(mutex, 0, 1))
    error("sem_init");

  if ((descriptors = (int *)malloc(count)) == NULL)
    error("malloc");

  for (i = 0; i < count; i++) {
    descriptors[i] = open(argv[i + 1], O_RDWR | 0664);
    if (descriptors[i] == -1)
      error("open");
  }

  pid = fork();

  act.sa_handler = handler;
  act.sa_mask = set;
  act.sa_flags = 0;

  sigaction(SIGINT, &act, NULL);

  if (pid == 0) {

    sem_wait(sem_a);
    tunnel(descriptors, count);
    sem_post(sem_b);

  } else if (pid > 0) {

    sem_wait(sem_b);
    printf("Press CTRL+C to start SIGINT event");
    sem_post(sem_a);

  } else {
    error("fork");
  }
}
