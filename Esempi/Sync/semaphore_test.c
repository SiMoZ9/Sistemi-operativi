#ifdef __unix__

#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/ipc.h>

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>l