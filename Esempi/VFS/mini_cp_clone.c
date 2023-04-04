#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define BUFFER_SIZE 1024 //copy 1024 bytes at each iteration

#define ABORT(){printf("Exit with error code: %d\n", errno);puts(strerror(errno)); exit(EXIT_FAILURE);}

int main(int argc, char **argv)
{
    int src, dest;
    ssize_t size, res;
    char tmp_buffer[BUFFER_SIZE];

    if(argc != 3)
        printf("usage: cpy src dest");

    src = open(argv[1], O_RDONLY);
    if(src == -1)
        ABORT();
    
    dest = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if(dest == -1)
        ABORT();

    do
    {
        /*reads 1024bytes from src to tmp_buffer*/
        size = read(src, tmp_buffer, BUFFER_SIZE);
        if(size == -1)
            ABORT();
        
        /*writes tmp_buffer to dest with size size*/
        res = write(dest, tmp_buffer, size);
        if(res == -1)
            ABORT();
    }while(size > 0); //do it until read() reads 0 bytes from src -> all records have been read

    close(src);
    close(dest);       
}
