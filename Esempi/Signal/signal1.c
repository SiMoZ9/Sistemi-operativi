#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <unistd.h>

void dummy(int arg) {
  system("cat signal1.c");
  kill(getpid(), SIGKILL);
}

int main(int argc, char **argv) {

  printf("Waiting for SIGINT...");
  while (1) {
    signal(SIGQUIT, &dummy);
  }
}
