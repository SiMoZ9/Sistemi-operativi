#ifdef __unix__

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

#else

#error "Cannot compile"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _abort(x){printf("%s", x); exit(EXIT_FAILURE);}

int main(int argc, char **argv)
{
	int fd;
	size_t size = 5;
	fd = open(argv[1], O_CREAT | O_WRONLY | O_TRUNC, 0664);
	if(fd == -1)
		_abort(strerror(errno));

	if(mmap(NULL, size*sizeof(size_t), PROT_WRITE, MAP_ANONYMOUS, fd, 0) == NULL)
		_abort(strerror(errno));

}

