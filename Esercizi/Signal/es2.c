/*
  Scrivere un programma in C in ambiente POSIX che:
    chiede ciclicamente all’utente l’inserimento di
    una nuova stringa.
    • per ogni nuova stringa inserita un gestore di interrupt
    (BREAK) può leggerne il contenuto per poi
    ristampare a schermo.
*/

#ifdef __unix__

#include <errno.h>
#include <signal.h>
#include <unistd.h>

#else

#error "Cannot compile on non-Unix systems"

#endif

#include <stdio.h>
#include <stdlib.h>

char *buffer;

void dummy(int arg) {
  fflush(stdout);
  printf("%s\n", buffer);
}

int main(void) {

  sigset_t set, old_set;

  buffer = (char *)malloc(100);
  struct sigaction act;
  struct sigaction old_act;

  // block signal before setup

  sigaddset(&set, SIGINT);
  sigprocmask(SIG_BLOCK, &set, &old_set);

  act.sa_handler = dummy;
  act.sa_mask = set;
  act.sa_flags = SA_SIGINFO;

  sigemptyset(&set);
  sigfillset(&set);
  sigaddset(&set, SIGINT);
  sigprocmask(SIG_UNBLOCK, &set, NULL);

  sigaction(SIGINT, &act, NULL);

  while (1) {
    printf("Inserisci qualcosa: ");
    fgets(buffer, 100, stdin);

    pause();
  }
}
