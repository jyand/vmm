#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NUM_BYTES 256
#define PAGE_SIZE NUM_BYTES
#define FRAME_SIZE NUM_BYTES

/* number of lines, not bytes */
int FileSize(const char *fname) {
        FILE *f = fopen(fname, "rt") ;
        int fsize = 0 ;
        do {
                int fch = fgetc(f) ;
                if (fch == '\n') {
                        ++fsize ;
                }
        } while (!feof(f)) ;
        fclose(f) ;
        return fsize ;
}

long *GetLogicalAddr(const char *fname) {
        int length = FileSize(fname) ;
        FILE *f = fopen(fname, "rt") ;
        long *logical_addr  = malloc(length*sizeof(long)) ;
        for (int k = 0 ; k < length ; ++k) {
                fscanf(f, "%d", &(logical_addr[k])) ;
        }
        fclose(f) ;
        return logical_addr ;
}

int main(int argc, char **argv) {
        const char* filename = argv[1] ;
        if (argv[1] == NULL) {
                return -1 ;
        } else {
                long *la = GetLogicalAddr(filename) ;
                for (int k = 0 ; k < FileSize(filename) ; ++k) {
                        printf("%d\n", la[k]) ;
                }
                free(la) ;
        }
        return 0 ;
}
