/*
  SPECIFICATION TO BE IMPLEMENTED:
  Implementare una programma che riceva in input, tramite argv[], il nome
  di un file F. Il programa dovra' creare il file F e popolare il file
  con lo stream priveniente da standard-input. Il programma dovra' generare
  anche un ulteriore processo il quale dovra' riversare il contenuto  che
  viene inserito in F su un altro file denominato shadow_F, tale inserimento
  dovra' essere realizzato in modo concorrente rispetto all'inserimento dei dati
  su F.

  L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
  WinAPI) in modo tale che quando un qualsiasi processo (parent o child) venga
  colpito si dovra' immediatamente emettere su standard-output il contenuto del
  file che il processo child sta popolando.

  Qualora non vi sia immissione di input, l'applicazione dovra' utilizzare
  non piu' del 5% della capacita' di lavoro della CPU.
*/

#ifdef __linux__
#include <errno.h>
#include <fcntl.h>

#include "../lib/sem_tools.h"
#include <semaphore.h>
#include <signal.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#else
#error

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __error(msg)                                                           \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

char *buffer;
char *aux_buffer;

int sem_a;
int sem_b;

int fd;
int shadow_fd;

void handler(int dummy) {}

int main(int argc, char **argv) {

  pid_t pid;
  sigset_t set;

  int rd, wr;

  struct stat st;

  if (argc != 3) {
    printf("missing filename\n");
    fflush(stdout);
    exit(-1);
  }

  buffer = (char *)malloc(4096);
  aux_buffer = (char *)malloc(4096);
  if (buffer == NULL || aux_buffer == NULL)
    __error("malloc");

  fd = open(argv[1], O_CREAT | O_RDWR, 0666);
  shadow_fd = open(argv[2], O_CREAT | O_RDWR, 0666);
  if (fd == -1 || shadow_fd == -1)
    __error("open");

  sem_a = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666);
  if (sem_a == -1)
    __error("semget");

  if (semctl(sem_a, 0, SETVAL, 1) == -1 || semctl(sem_a, 1, SETVAL, 0) == -1)
    __error("semctl");

  pid = fork();
  if (pid < 0)
    __error("fork");

  if (pid == 0) {
    my_wait(sem_a, 0);
    printf("Child is executing\n");
    fflush(stdout);

    if (fgets(buffer, 4096, stdin) == NULL)
      __error("fgets");

    write(fd, buffer, 4096);
    my_post(sem_a, 1);
  } else {

    fstat(fd, &st);
    int filesize = st.st_size;

    my_wait(sem_a, 1);

    printf("Parent is writing\n");
    fflush(stdout);

    while (filesize != 0) {

      int chunk = 1000;
      chunk = (filesize < chunk) ? filesize : chunk;

      rd = read(fd, aux_buffer, chunk);
      printf("cotoletta %s\n", aux_buffer);
      if (rd == -1)
        __error("read");

      wr = write(shadow_fd, aux_buffer, chunk);

      filesize -= wr;
    }

    my_post(sem_a, 0);
  }

  free(buffer);
  free(aux_buffer);
}
