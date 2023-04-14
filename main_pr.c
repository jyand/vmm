#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NUM_BYTES 256
#define PAGE_SIZE NUM_BYTES
#define FRAME_SIZE NUM_BYTES

struct PageFrame {
        int pagenum ;
        int framenum ;
} ;

struct PageFrame TLBuffer[16] ;
struct PageFrame PageTable[256] ;

long PhysicalMem[FRAME_SIZE][NUM_BYTES] ;

int frame_index = 0 ;
int tlb_index = 0 ;
int pgtbl_index = 0 ;

int tlb_hit_count = 0 ;
int page_fault_count = 0 ;

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

void ReadBackingStore(int pagenum) {
        signed char buffer[256] ;
        FILE *f = fopen("BACKING_STORE.bin", "rb") ;
        fseek(f, NUM_BYTES*pagenum, SEEK_SET) ;
        fread(buffer, sizeof(char), NUM_BYTES, f) ;
        fclose(f) ;
        if (frame_index < FRAME_SIZE) {
                for (int k = 0 ; k < NUM_BYTES ; ++k) {
                        PhysicalMem[frame_index][k] = buffer[k] ;
                }
                PageTable[pgtbl_index].pagenum = pagenum ;
                PageTable[pgtbl_index].framenum = frame_index ;
                frame_index++ ;
                pgtbl_index ++ ;
        } else {
                int k ;
                for (k = 0 ; k < FRAME_SIZE - 1 ; ++k) {
                        for (int j = 0 ; j < NUM_BYTES ; ++j) {
                                PhysicalMem[k][j] = PhysicalMem[k+1][j] ;
                        }
                        PageTable[k] = PageTable[k+1] ;
                }
                for (int j = 0 ; j < NUM_BYTES ; ++j) {
                        PhysicalMem[k][j] = buffer[j] ; 
                }
                PageTable[k].pagenum = pagenum ;
                PageTable[k].framenum = frame_index - 1 ;
        }
}

void TLBInsert(int pagenum, int framenum) {
        if (tlb_index < 16) {
                TLBuffer[tlb_index].pagenum = pagenum ;
                TLBuffer[tlb_index].framenum = framenum ;
                tlb_index++ ;
        } else {
                int k ;
                for (k = 0 ; k < 15 ; ++k) {
                        TLBuffer[k] = TLBuffer[k+1] ;
                }
                TLBuffer[k].pagenum = pagenum ;
                TLBuffer[k].framenum = framenum ;
        }
}

long TranslatePhysicalAddr(long log_addr) {
    unsigned char page = (log_addr >> 8) & 0xFF ;
    unsigned char offset = log_addr & 0xFF ;
    int frame = -1 ;
    for (int k = 0 ; k < tlb_index ; ++k) {
        if (TLBuffer[k].pagenum == page) {
            frame = TLBuffer[k].framenum ;
            tlb_hit_count++ ;
            break ;
        }
    }
    if (frame == -1) {
        for (int k = 0 ; k < pgtbl_index ; ++k) {
            if (PageTable[k].pagenum == page) {
                frame = PageTable[k].framenum ;
                break ;
            }
        }
        if (frame == -1) {
            ReadBackingStore(page) ;
            page_fault_count++ ;
            frame = frame_index - 1 ;
            PageTable[pgtbl_index].pagenum = page ;
            PageTable[pgtbl_index].framenum = frame_index - 1 ;
            pgtbl_index++ ;
        }
        TLBInsert(page, frame) ;
    }
    long phys_addr = PAGE_SIZE*frame + offset ;
    return phys_addr ;
}

int main(int argc, char **argv) {
        const char* filename = argv[1] ;
        if (argv[1] == NULL) {
                return -1 ;
        } else {
                long *la = GetLogicalAddr(filename) ;
                for (int k = 0 ; k < FileSize(filename) ; ++k) {
                        long temp = TranslatePhysicalAddr(la[k]) ;
                        printf("%d\n", temp) ;
                }
                free(la) ;
        }
        return 0 ;
}
