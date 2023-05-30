#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void gestione(int dummy1, siginfo_t *info, void *dummy2) {
  unsigned int address;
  address = (unsigned int)info->si_addr;
  printf("Segfault at address 0x%x\n", address);

  fflush(stdout);
  exit(1);
}

int main(void) {
  sigset_t set;

  struct sigaction act;

  act.sa_sigaction = gestione;
  act.sa_mask = set;
  act.sa_flags = SA_SIGINFO;
  act.sa_restorer = NULL;

  char *b;

  sigaction(SIGSEGV, &act, NULL);

  for (int i = 0; i < 10; i++) {
    b[i] = 'a';
  }
}
