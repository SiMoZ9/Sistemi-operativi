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

int sem_m; // semaforo di mutua esclusione
int sem_a; // semaforo di regolazione agli accessi

int fd;
int num_td;

char *buffer;

ssize_t wd;

void my_post(int semid, int semnum) {
  struct sembuf ops;

  ops.sem_op = 1;
  ops.sem_num = semnum;
  ops.sem_flg = 0;

  if (semop(semid, &ops, 1) == -1)
    error("semop");
}

void my_wait(int semid, int semnum) {
  struct sembuf ops;

  ops.sem_op = -1;
  ops.sem_num = semnum;
  ops.sem_flg = 0;

  if (semop(semid, &ops, 1) == -1)
    error("semop");
}

void *routine(void *args) {
  long i = (long)args;

  my_wait(sem_a, i);

  printf("Hi i'm thread %lu", i);
  fflush(stdout);

  while (1) {

    my_wait(sem_m, 0);

    // wd = write(fd, buffer, 5);

    if (wd == -1)
      error("write");

    my_post(sem_m, 0);
  }
  if (i < num_td - 1) {
    my_post(sem_a, i + 1);
  } else {
    my_post(sem_a, num_td - i + 1);
  }
}

int main(int argc, char **argv) {

  int i;
  pthread_t tid;

  num_td = argc;
  buffer = (char *)malloc(4096);

  if (argc < 2) {
    printf("Usage: ./prog F1 F2 ... [Fn]");
    exit(-1);
  }

  /******************************************************************************************************************/

  sem_m = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
  sem_a = semget(IPC_PRIVATE, num_td, IPC_CREAT | 0666);

  if (sem_a == -1 || sem_m == -1)
    error("semget");

  if (semctl(sem_m, 0, SETVAL, 1) == -1)
    error("semctl");

  semctl(sem_a, 1, SETVAL, 1);

  for (i = 2; i < num_td; i++) {
    if (semctl(sem_a, i, SETVAL, 0) == -1)
      error("semctl");
  }

  /******************************************************************************************************************/

  for (i = 0; i < num_td; i++) {
    if (pthread_create(&tid, NULL, routine, (void *)i) != 0)
      error("pthread_create");
  }

  /******************************************************************************************************************/
  for (i = 1; i < num_td; i++) {
    fd = open(argv[i], O_CREAT | O_TRUNC | O_RDWR, 0666);

    if (fd < 0)
      error("open");
  }

  while (1) {
    pthread_join(tid, NULL);
    close(fd);
  }
}
