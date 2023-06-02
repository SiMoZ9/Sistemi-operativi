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

int num_td;

char *buffer;
int *files_back_arr;
int *files;

int sem;

ssize_t wd;

#define BUFFER_SIZE 5

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

void handler(int dummy) {
  int res;
  int i;
  int turn;

  turn = 0;

  while (1) {
    res = 0;
    if ((res = read(files_back_arr[turn + 1], buffer, 5)) == -1)
      error("read");

    for (i = 0; i < res; i++)
      putchar(buffer[i]);
    fflush(stdout);

    if (res < 5) {
      printf("\nDone\n");
      fflush(stdout);
      exit(EXIT_SUCCESS);
    }

    turn = (turn + 1) % (num_td);
  }
}

void *routine(void *args) {
  long i = (long)args;

  printf("thread %lu setup\n", i);
  fflush(stdout);

  while (1) {
    my_wait(sem, i);
    printf("thread %lu is operating\n", i);
    fflush(stdout);

    if (write(files[i], buffer, 5) == -1)
      error("write");
    my_post(sem, 0);
  }
}

int main(int argc, char **argv) {
  pthread_t tid;
  long i;
  int turn;

  sigset_t set;
  struct sigaction act;

  num_td = argc;
  buffer = (char *)malloc(4096);

  files_back_arr = (int *)malloc(sizeof(int) * argc);
  files = (int *)malloc(sizeof(int) * argc);

  sem = semget(IPC_PRIVATE, argc, IPC_CREAT | 0666);

  if (sem == -1)
    error("semget");

  if (semctl(sem, 0, SETVAL, 1) == -1)
    error("semctl");

  for (i = 1; i < argc; i++) {
    if (semctl(sem, i, SETVAL, 0) == -1)
      error("semctl");
  }

  sigemptyset(&set);
  sigfillset(&set);
  sigaddset(&set, SIGINT);

  sigprocmask(SIG_UNBLOCK, &set, NULL);

  act.sa_handler = handler;
  act.sa_mask = set;
  act.sa_flags = SA_SIGINFO;
  act.sa_restorer = 0;

  for (i = 1; i < argc; i++) {

    files[i] = open(argv[i], O_CREAT | O_RDWR | O_TRUNC, 0666);
    files_back_arr[i] = open(argv[i], O_CREAT | O_RDWR | O_TRUNC, 0666);

    if (files[i] == -1 || files_back_arr[i] == -1)
      error("open");
  }
  for (i = 1; i < num_td; i++) {
    if (pthread_create(&tid, NULL, routine, (void *)i))
      error("pthread_create");
  }

  sigaction(SIGINT, &act, NULL);

  turn = 0;

  while (1) {

    my_wait(sem, 0);

    for (i = 0; i < 5; i++)
      buffer[i] = getchar();

    my_post(sem, turn + 1);

    turn = (turn + 1) % (argc - 1);
  }

  free(buffer);
  free(files_back_arr);

  for (i = 1; i < argc; i++)
    close(files[i]);

  free(files);
}
