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
int fs;

char *buffer;

pthread_mutex_t *mutex;
pthread_mutex_t *access_m; // regola gli accessi

ssize_t wd;

#define BUFFER_SIZE 5

#ifdef sem_compile

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

#else

void *routine(void *args) {
  long i = (long)args;

  int chunk = BUFFER_SIZE;

  printf("thread %lu setup\n", i);
  fflush(stdout);

  printf("thread %lu is operating\n", i);
  fflush(stdout);

  while (1) {
    pthread_mutex_lock(access_m + i);
    wd = write(fd, buffer, chunk + 5);

    if (i >= num_td)
      i = 0;
    pthread_mutex_unlock(mutex + i + 1);
    pthread_mutex_unlock(access_m + i + 1);
  }
}

int main(int argc, char **argv) {
  pthread_t tid;
  long i;

  num_td = argc;
  buffer = (char *)malloc(4096);

  // mutex init

  mutex = (pthread_mutex_t *)malloc(4000);
  access_m = (pthread_mutex_t *)malloc(4000);

  for (i = 0; i < num_td; i++) {
    if (pthread_mutex_init(access_m + i, NULL))
      error("pthread_mutex_init");

    pthread_mutex_init(mutex + i, NULL);
    // pthread_mutex_lock(access_m + i);
  }

  printf("Scrivi qualcosa: ");
  fs = read(STDIN_FILENO, buffer, 4096);
  printf("%d\n\n\n", fs);
  // threads creation

  for (i = 1; i < num_td; i++) {
    if (pthread_create(&tid, NULL, routine, (void *)i))
      error("pthread_create");

    fd = open(argv[i], O_CREAT | O_TRUNC | O_WRONLY, 0666);
    if (fd == -1)
      error("open");
  }

  pthread_mutex_lock(mutex);

  pthread_join(tid, NULL);

  pthread_mutex_unlock(access_m);

  free(mutex);
  free(access_m);
  free(buffer);
}
#endif
