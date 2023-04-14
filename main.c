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
                        // physical memory
                }
                PageTable[pgtbl_index].pagenum = pagenum ;
                PageTable[pgtbl_index].framenum = frame_index ;
                frame_index++ ;
                pgtbl_index ++ ;
        } else {
                for (int k = 0 ; k < FRAME_SIZE ; ++k) {
                        for (int j = 0 ; j < NUM_BYTES ; ++j) {
                                // physical memory something...
                        }
                        PageTable[k] = PageTable[k+1] ;
                }
                for (int k = 0 ; k < NUM_BYTES ; ++k) {
                        // phys mem blah blah
                }
                PageTable[k].pagenum = pagenum ;
                PageTable[k].framenum = frame_index - 1 ;
        }
}

void TLBInsert(int pagenum, int framenum) {
        int count ;
        for (count = 0 ; count < tlb_index ; ++count) {
                if (TLBuffer[count].pagenum == pagenum) {
                        break ;
                }
        }
        if (count == tlb_index) {
                if (tlb_index < 16) {
                        TLBuffer[tlb_index].pagenum = pagenum ;
                        TLBuffer[tlb_index].framenum = framenum ;
                } else {
                        for (int k = 0 ; k < 15 ; ++k) {
                                TLBuffer[k] = TLBuffer[k+1] ;
                        }
                        TLBuffer[15].pagenum = pagenum ;
                        TLBuffer[15].framenum = framenum ;
                }
        } else {
                for (int k = 0 ; k < 15 ; ++k) {
                        TLBuffer[k] = TLBuffer[k+1] ;
                }
                if (tlb_index < 16) {
                        TLBuffer[tlb_index].pagenum = pagenum ;
                        TLBuffer[tlb_index].framenum = framenum ;
                } else {
                        TLBuffer[tlb_index-1].pagenum = pagenum ;
                        TLBuffer[tlb_index-1].framenum = framenum ;
                }
        }
        if (tlb_index < 16) {
                tlb_index++ ;
        }
}

void GetPage(long log_addr) {
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
                ReadBackingStore(page) ;
                page_fault_count++ ;
                frame = frame_index - 1 ;
        }
        TLBInsert(page, frame) ;
        printf("%d\n", log_addr) ;
        printf("%d\n", PAGE_SIZE*frame + offset) ;
}

/*
--- Function to translate from logical to physical ---

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
*/

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
