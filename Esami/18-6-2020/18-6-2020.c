/*
  Scrivere una funzione in linguaggio C (Posix o WinAPI a scelta dello studente)
  con la seguente interfaccia "void channels(int descriptors[])" tale che,
  ricevendo in input l'array null-terminated di file descriptors "descriptors[]"
  crei, per ogni elemento non nullo dell'array, una nuova pipe ed un thread che
  riversi lo stream associato all'elemento dell'array sul canale di scrittura
  della pipe. Inoltre, la funzione "int channels(int  descriptors[])" dovra'
  attivare la gestione del segnale SIGINT nel caso Posix (oppure dell'evento
  CTRL_C_EVENT nel caso WinAPI) in modo tale che il gestore del segnale esegua
  la lettura di tutti gli stream associati alle pipe e riversi il contenuto
  sullo standard output. E' compito dello studente produrre anche il codice C
  del gestore.
*/

#ifdef __linux__

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

#define error(msg)                                                             \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

#else
#error

#endif

#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 1000

int *fds;
int *pipe_desc[2];
int fd[2];
int num_thread;
ssize_t wd, rd;

pthread_mutex_t *mutex_a;
pthread_mutex_t *mutex_b;

typedef struct __args_t {
  int me;
  int *descriptors;
} td_args_t;

struct stat st;

sigset_t set;
struct sigaction act;

void handler(int dummy) {

  char handler_buffer[BUFFER_SIZE];

  close(fd[1]);
  int i;
  for (i = 0; i < num_thread; i++) {
    while ((rd = read(pipe_desc[i][0], handler_buffer,
                      sizeof(handler_buffer) - 1)) > 0) {
      handler_buffer[rd] = '\0';

      fflush(stdout);
      write(1, handler_buffer, sizeof(handler_buffer));
    }
  }

  close(fd[0]);
  exit(EXIT_SUCCESS);
}

void *channels(void *args) {
  td_args_t *arg = (td_args_t *)args;

  long me = arg->me;
  char buffer[BUFFER_SIZE];

  fstat(arg->descriptors[me], &st);
  int filesize = st.st_size;

  printf("Thread %d setup\n", arg->me);
  fflush(stdout);

  if (pipe(fd) == -1)
    error("pipe");

  while (1) {

    pthread_mutex_lock(mutex_a + me);

    printf("thread %d is executing\n", arg->me);
    fflush(stdout);

    int chuck = BUFFER_SIZE;
    chuck = (filesize < chuck) ? filesize : chuck;

    rd = read(arg->descriptors[me], buffer, BUFFER_SIZE);
    if (rd == -1)
      error("read");
    close(fd[0]);

    printf("Writing\n");

    wd = write(fd[1], buffer, chuck);

    if (wd == -1)
      error("write");

    filesize -= wd;

    close(fd[1]);

    pthread_mutex_unlock(mutex_b + me);
  }
}

int main(int argc, char **argv) {

  int i;
  pthread_t tid;

  td_args_t *tid_arg;

  if (argc < 2) {
    printf("too few args");
    return -1;
  }

  num_thread = argc - 1;
  mutex_a = malloc(num_thread * sizeof(pthread_mutex_t));
  mutex_b = malloc(num_thread * sizeof(pthread_mutex_t));

  *pipe_desc = (int *)malloc(sizeof(int *) * num_thread);

  if (mutex_a == NULL || mutex_b == NULL)
    error("malloc");

  if (pthread_mutex_init(mutex_a, NULL) || pthread_mutex_init(mutex_b, NULL))
    error("pthread_mutex_init");

  pthread_mutex_lock(mutex_a);

  sigemptyset(&set);
  sigfillset(&set);
  sigaddset(&set, SIGINT);
  sigprocmask(SIG_UNBLOCK, &set, NULL);

  for (i = 0; i < num_thread; i++) {
    tid_arg = (td_args_t *)malloc(sizeof(td_args_t));
    tid_arg->descriptors = (int *)malloc(sizeof(int) * num_thread);
    tid_arg->descriptors[i] = open(argv[i + 1], O_RDWR | 0664);
    if (tid_arg->descriptors[i] == -1)
      error("open");

    tid_arg->me = i;

    if (pthread_create(&tid, NULL, channels, (void *)(tid_arg)) != 0)
      error("pthread_create");
  }

  int turn = 0;

  act.sa_handler = handler;
  act.sa_mask = set;
  act.sa_flags = 0;

  sigaction(SIGINT, &act, NULL);

  while (1) {
  }
}
