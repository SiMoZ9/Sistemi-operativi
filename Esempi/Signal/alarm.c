#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <unistd.h>

void dummy(int foo) {

  alarm(1);
  fflush(stdout);
  printf("Scriviii\n");
}

void dummy2(int n) {

  fflush(stdout);
  system("ls -la ~/");
  exit(1);
}
int main(void) {

#ifdef prog1
  int n = 0;
  int buff[100];

  // alarm(5);
  signal(SIGALRM, &dummy); // dopo il timer esegue dummy

  while (n <= 0) {
    printf("Scrivi qualcosa: ");
    alarm(2);

    if ((n = read(STDIN_FILENO, buff, 10)) < 0)
      perror("read");
    alarm(0);
  }

#endif

#ifdef prog2

  alarm(5);
  while (1)
    signal(SIGALRM, &dummy2);

#endif
}
