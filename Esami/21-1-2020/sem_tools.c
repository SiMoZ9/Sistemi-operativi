#include <sys/sem.h>

#include "sem_tools.h"

int my_wait(int semid, int semnum) {
  struct sembuf sops;

  sops.sem_op = -1;
  sops.sem_num = semnum;
  sops.sem_flg = 0;

  if (semop(semid, &sops, 1) == -1) {
    return 1;
  }

  else
    return 0;
}

int my_post(int semid, int semnum) {
  struct sembuf sops;

  sops.sem_op = 1;
  sops.sem_num = semnum;
  sops.sem_flg = 0;

  if (semop(semid, &sops, 1) == -1) {
    return 1;
  }

  else
    return 0;
}
