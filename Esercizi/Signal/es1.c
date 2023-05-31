/*
  Scrivere un programma in C in ambiente UNIX che
  permette a due processi di scrivere su standard
  output in alternanza stretta.
  • La coordinazione tra i due processi deve essere
  effettuata per mezzo di appositi segnali (e.g.
  SIGUSR1) al fine di comunicare al processo
  concorrente la terminazione della propria attività e
  quindi di dare ad esso la possibilità di effettuare le
  proprie attività
*/

#ifndef __unix__

#error "Cannot compile on non-Unix systems"

#else

#include <errno.h>
#include <signal.h>
#include <sys/sem.h>
#include <unistd.h>

#endif

#include <stdio.h>
#include <stdlib.h>

char *buffer;

pid_t pid1, pid2;

void writer(int dummy) {

  printf("Esegue il pid %d: %d\n", pid1, dummy);
  sleep(1);
  kill(pid2, SIGUSR1);
}

int main() {
  struct sigaction act;
  struct sigaction old_act;

  sigset_t set;
  sigset_t old_set;

  sigfillset(&set);
  sigprocmask(SIG_BLOCK, &set, &old_set);

  buffer = (char *)malloc(100);

  act.sa_handler = writer;
  act.sa_mask = set;

  sigaction(SIGUSR1, &act, &old_act);

  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);
  sigaddset(&set, SIGINT);
  sigprocmask(SIG_UNBLOCK, &set, NULL);

  if ((pid2 = fork()) == -1)
    exit(-1);
  else if (pid2 == 0) {
    /* Child Process */
    pid1 = getpid();
    pid2 = getppid();

    while (1) {
      pause();
    }
  } else {
    /* Parent Process */
    pid1 = getpid();

    while (raise(SIGUSR1))
      ;

    while (1) {
      pause();
    }
  }

  sigaction(SIGUSR1, &old_act, NULL);
  sigprocmask(SIG_SETMASK, &old_set, NULL);
}
