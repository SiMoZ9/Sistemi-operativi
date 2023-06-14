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
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
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

int sem_a;
int shmid;

int fd;
int fds;
int rd, wr;
char **args;

int child_pid;

void handler(int dummy) {

  if (getpid() == child_pid)
    kill(child_pid, SIGTERM);

  close(fd);
  close(fds);

#ifdef debug

  int size = lseek(fds, 0, SEEK_END);
  printf("%d\n", size);
#endif

  printf("\n\n\nhandler activated\n\n");

  fds = open(args[2], O_RDONLY);

  char aux[100];

  while ((rd = read(fds, aux, 100)) > 0)
    printf("%s", aux);

  exit(1);
}

int main(int argc, char **argv) {

  pid_t pid;
  sigset_t set;
  struct sigaction act;

  if (argc != 3) {
    printf("missing filename\n");
    fflush(stdout);
    exit(-1);
  }

  args = (char **)malloc(sizeof(char *) * (argc));

  args = argv;

  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigprocmask(SIG_UNBLOCK, &set, NULL);

  fd = open(argv[1], O_CREAT | O_TRUNC | O_RDWR, 0666);
  fds = open(argv[2], O_CREAT | O_TRUNC | O_RDWR, 0666);
  if (fd == -1 || fds == -1)
    __error("open");

  shmid = shmget(IPC_PRIVATE, 4096, IPC_CREAT | 0666);
  if (shmid == -1)
    __error("shmget");

  buffer = shmat(shmid, NULL, SHM_W | SHM_R);
  if (buffer == (void *)-1)
    __error("shmat");

  sem_a = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666);
  if (sem_a == -1)
    __error("semget");

  if (semctl(sem_a, 0, SETVAL, 1) == -1 || semctl(sem_a, 1, SETVAL, 0) == -1)
    __error("semctl");

  pid = fork();
  if (pid < 0)
    __error("fork");

  act.sa_mask = set;
  act.sa_flags = SA_SIGINFO;
  act.sa_handler = handler;

  sigaction(SIGINT, &act, NULL);

  while (1) {

    if (pid == 0) {
      child_pid = getpid();
      my_wait(sem_a, 0);
      printf("Child is executing\n");
      fflush(stdout);

      if (fgets(buffer, 4096, stdin) == NULL)
        __error("fgets");

      wr = write(fd, buffer, 4096);
      if (wr == -1)
        __error("write");

      my_post(sem_a, 1);

    } else {
      my_wait(sem_a, 1);

      printf("Parent is executing %s\n", buffer);
      fflush(stdout);

      wr = write(fds, buffer, 100);
      if (wr == -1)
        __error("write");

      my_post(sem_a, 0);
    }
  }
}
