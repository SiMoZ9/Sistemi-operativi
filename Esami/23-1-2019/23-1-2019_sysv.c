/*
  SPECIFICATION TO BE IMPLEMENTED:
  Implementare una programma che riceva in input, tramite argv[], il nome
  di un file F ed N stringhe S_1 .. S_N (con N maggiore o uguale
  ad 1.
  Per ogni stringa S_i dovra' essere attivato un nuovo thread T_i, che fungera'
  da gestore della stringa S_i.

  Il main thread dovra' leggere indefinitamente stringhe dallo standard-input.
  Ogni nuova stringa letta dovra' essere comunicata a tutti i thread T_1 .. T_N
  tramite un buffer condiviso, e ciascun thread T_i dovra' verificare se tale
  stringa sia uguale alla stringa S_i da lui gestita.

  In caso positivo, ogni carattere della stringa immessa dovra' essere
  sostituito dal carattere '*'. Dopo che i thread T_1 .. T_N hanno analizzato la
  stringa, ed eventualmente questa sia stata modificata, il main thread dovra'
  scrivere tale stringa (modificata o non) su una nuova linea del file F.

  In altre parole, la sequenza di stringhe provenienti dallo standard-input
  dovra' essere riportata su file F in una forma 'epurata'  delle stringhe S1 ..
  SN, che verranno sostituite da strighe  della stessa lunghezza costituite
  esclusivamente da sequenze del carattere '*'.

  Inoltre, qualora gia' esistente, il file F dovra' essere troncato (o
  rigenerato) all'atto del lancio dell'applicazione.

  L'applicazione dovra' gestire il segnale SIGINT (o CTRL_C_EVENT nel caso
  WinAPI) in modo tale che quando il processo venga colpito esso dovra'
  riversare su standard-output il contenuto corrente del file F.

  Qualora non vi sia immissione di input, l'applicazione dovra' utilizzare
  non piu' del 5% della capacita' di lavoro della CPU.
*/

#ifdef __linux__

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <unistd.h>

#else

#error "Cannot compile on non-Linux systems"

#endif

// always includes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define error(msg)                                                             \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0);

#define STR_MAX_SIZE 4096

typedef struct __args {
  int me;
  char *str;
} td_args;

int fd;

int num_td;

int wr;
int rd;

char *fname;
char *buffer;

void my_post(int sem_id, int semnum) {
  struct sembuf ops;

  ops.sem_op = 1;
  ops.sem_num = semnum;
  ops.sem_flg = 0;

  if (semop(sem_id, &ops, 1) == -1)
    error("semop");
}

void my_wait(int sem_id, int semnum) {
  struct sembuf ops;

  ops.sem_op = -1;
  ops.sem_num = semnum;
  ops.sem_flg = 0;

  if (semop(sem_id, &ops, 1) == -1)
    error("semop");
}
void *td_func(void *arg) {

  td_args *args = (td_args *)arg;
  long i = args->me;
  size_t j;

  printf("thread %lu setup done\n", i);

  fflush(stdout);

  while (1) {
    my_wait(wr, args->me);

    if (strcmp(buffer, args->str) == 0) {
      strcpy(buffer, "*");

      for (j = 0; j < (strlen(args->str) - 1); j++)
        strcat(buffer, "*");
    }

    my_post(rd, args->me);
  }
}

void handler(int dummy) {

  char cmd[4096];
  printf("\n\n\n");
  sprintf(cmd, "cat %s", fname);
  system(cmd);

  kill(getpid(), SIGTERM);
}

int main(int argc, char **argv) {

  int i;
  pthread_t td;

  sigset_t set;
  struct sigaction act;

  if (argc < 3) {
    printf("Usage: ./prog F S_1 [S_2] ... [S_n]");
    exit(EXIT_FAILURE);
  }

  num_td = argc - 2; // filename and F are not included
  buffer = (char *)malloc(sizeof(char) * STR_MAX_SIZE);
  fname = (char *)malloc(strlen(argv[1] + 1));

  wr = semget(IPC_PRIVATE, num_td, 0666 | IPC_CREAT);
  rd = semget(IPC_PRIVATE, num_td, 0666 | IPC_CREAT);

  if (wr == -1 || rd == -1)
    error("semget");

  for (i = 0; i < num_td; i++) {
    semctl(wr, i, SETVAL, 0);
    semctl(rd, i, SETVAL, 1);
  }

  fd = open(argv[1], O_CREAT | O_TRUNC | O_RDWR, 0664);
  if (fd == -1)
    error("open");

  fname = argv[1];

  sigemptyset(&set);
  sigfillset(&set);
  sigaddset(&set, SIGINT);
  sigprocmask(SIG_BLOCK, &set, NULL);

  for (i = 0; i < num_td; i++) {

    td_args *current = (td_args *)malloc(sizeof(td_args));
    current->me = i;
    current->str = argv[i + 2];

    if (pthread_create(&td, NULL, td_func, (void *)current))
      error("pthread_create");
  }

  FILE *fp = fdopen(fd, "r+");

  sigprocmask(SIG_UNBLOCK, &set, NULL);

  act.sa_mask = set;
  act.sa_handler = handler;
  act.sa_flags = SA_SIGINFO;

  sigaction(SIGINT, &act, NULL);

  strcpy(buffer, "\0");
  while (1) {
    for (i = 0; i < num_td; i++)
      my_wait(rd, i);

    if (strcmp(buffer, "\0") != 0) {
      fprintf(fp, "%s\n", buffer);
      fflush(fp);
    }

    while (scanf("%s", buffer) <= 0)
      ;
    for (i = 0; i < num_td; i++)
      my_post(wr, i);
  }
}
