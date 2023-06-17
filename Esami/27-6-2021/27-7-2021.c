/*
  SPECIFICATION TO BE IMPLEMENTED:
  Implementare una programma che riceva in input, tramite argv[], il nome
  di un file F e un insieme di N stringhe (con N almeno pari ad 1).

  Il programa dovra' creare il file F e popolare il file con le stringhe
  provenienti da standard-input. Ogni stringa dovra' essere inserita su una
  differente linea del file.

  L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel
  caso WinAPI) in modo tale che quando il processo venga colpito si dovranno
  generare N processi concorrenti ciascuno dei quali dovra' analizzare il
  contenuto del file F e verificare, per una delle N stringhe di input, quante
  volte la inversa di tale stringa sia presente nel file.

  Il risultato del controllo dovra' essere comunicato su standard output tramite
  un messaggio.

  Quando tutti i processi avranno completato questo controllo, il contenuto del
  file F dovra' essere inserito in "append" in un file denominato "backup" e poi
  il file F dova' essere troncato.

  Qualora non vi sia immissione di input o di segnali, l'applicazione dovra'
  utilizzare non piu' del 5% della capacita' di lavoro della CPU.
*/

#ifdef __linux__

#include "../lib/sem_tools.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <unistd.h>

#include <sys/stat.h>

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
  } while (0)

char buffer[1024];
char rd_buff[1024];

char **buffer_strings;

char *filename;

long num_proc;

int fd;
FILE *backup;
int wr, rd, rd_2;

int sem;
int msg;

int count;
int found;

char *strrev(char *str) {
  char *p1, *p2;

  if (!str || !*str)
    return str;
  for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2) {
    *p1 ^= *p2;
    *p2 ^= *p1;
    *p1 ^= *p2;
  }
  return str;
}

void handler(int dummy) {

  close(fd);

  int i;
  pid_t pid;

  int size;

  found = 0;

  fd = open(filename, O_RDONLY, 0666);
  struct stat st;
  stat(filename, &st);

  lseek(fd, 0, SEEK_CUR);

  for (i = 0; i < count; i++) {
    pid = fork();

    size = st.st_size;

    if (pid == 0) {
      my_wait(sem, i);

      rd = read(fd, rd_buff, size);
      printf("\n\nproc %d read %s", i, rd_buff);
      fflush(stdout);

      if (rd_buff == strrev(buffer_strings[i])) {
        found++;
      }
    }
    my_post(sem, i + 1);
  }

  exit(1);
}

int main(int argc, char **argv) {

  sigset_t set;
  struct sigaction act;

  if (argc != 3) {
    printf("usage: ./prog filename n");
    exit(EXIT_FAILURE);
  }

  num_proc = strtol(argv[2], NULL, 10);
  if (num_proc < 1) {
    printf("number arg < 1");
    exit(EXIT_FAILURE);
  }

  filename = (char *)malloc(sizeof(char *));
  filename = argv[1];

  buffer_strings = (char **)malloc(sizeof(char *) * num_proc);

  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigprocmask(SIG_BLOCK, &set, NULL);

  sem = semget(IPC_PRIVATE, num_proc, IPC_CREAT | 0666);
  if (sem == -1)
    error("semget");

  if (semctl(sem, 0, SETVAL, 1) == -1)
    error("semctl");

  for (int i = 1; i < num_proc; i++) {
    if (semctl(sem, i, SETVAL, 0) == -1)
      error("semctl");
  }

  if ((fd = open(argv[1], O_CREAT | O_TRUNC | O_RDWR, 0666)) == -1)
    error("open");

  sigprocmask(SIG_UNBLOCK, &set, NULL);

  act.sa_mask = set;
  act.sa_flags = SA_SIGINFO;
  act.sa_handler = handler;

  sigaction(SIGINT, &act, NULL);

  count = 0;

  while (1) {

    fgets(buffer, sizeof(buffer), stdin);

    if ((wr = write(fd, buffer, sizeof(buffer))) == -1)
      error("fgets");

    buffer_strings[count] = strndup(buffer, sizeof(buffer));
    count++;
  }
}
