/*
    • Nei sistemi operativi UNIX, /dev/urandom è un
    dispositivo a caratteri virtuale in grado di generare
    numeri casuali. Nello specifico, l’operazione di
    lettura dal relativo file produce byte casuali.
    • Scrivere un programma C che:
    • prende come parametri da linea di comando: un numero
    N e una stringa S da usare come nome del file da
    creare;
    • crea un file S contenente N byte randomici;
    • utilizza il dispositivo /dev/random come sorgente di
    numeri pseudo-casuali.
*/

#ifdef __unix__

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#else
    #error "Cannot compile on non-unix systems"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _abort(x){printf("%s", x); exit(EXIT_FAILURE);}
#define BUFFER_SIZE 1000

int main(int argc, char **argv){
    int src, dst;
    ssize_t rd, wr;

    char buffer[BUFFER_SIZE];

    if(argc != 3)
        _abort("Usage: ./es1 <number_of_shorts> <file_name>");

    //Open /dev/urandom for reading

    long filesize = atol(argv[1]) * sizeof(short);

    src = open("/dev/urandom", O_RDONLY);
    if(src == -1)
        _abort(strerror(errno));

    dst = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0660);
    if(dst == -1)
        _abort(strerror(errno));

    while(filesize != 0)
    {
        int chuck = BUFFER_SIZE;
        chuck = (filesize < chuck) ? filesize : chuck;

        rd = read(src, buffer, BUFFER_SIZE);
        if(rd == -1)
            _abort(strerror(errno));
        
        wr = write(dst, buffer, chuck);
        if(wr == -1)
            _abort(strerror(errno));

        printf("Written %d bytes\n", chuck);

        filesize -= wr;
    }
}