#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

const int PHYS_MEM_SIZE = 256;
const int BUFFER_SIZE = PHYS_MEM_SIZE;
const int PAGE_SIZE = PHYS_MEM_SIZE;
const int TLB_SIZE = 16;

int printIndex;
int printValue;

struct TLB
{
    unsigned char TLBpage[TLB_SIZE];
    unsigned char TLBframe[TLB_SIZE];
    int ind;
};
 
int readDisk(int pageNum, char* PM, int* OF) {
    char buffer[PHYS_MEM_SIZE];

    FILE* backingStore = fopen("BACKING_STORE.bin", "rb");
    if (backingStore == NULL) {
        printf("ERROR: Failed to open BACKING_STORE.bin\n");
        exit(EXIT_FAILURE);
    }

    if (fseek(backingStore, pageNum * PHYS_MEM_SIZE, SEEK_SET) != 0) {
        printf("ERROR: Cannot seek to page %d in BACKING_STORE.bin\n", pageNum);
        exit(EXIT_FAILURE);
    }

    if (fread(buffer, sizeof(char), PHYS_MEM_SIZE, backingStore) != PHYS_MEM_SIZE) {
        printf("ERROR: Cannot read page %d from BACKING_STORE.bin\n", pageNum);
        exit(EXIT_FAILURE);
    }

    fclose(backingStore);

    int i;
    for (i = 0; i < PHYS_MEM_SIZE; i++) {
        PM[(*OF * PHYS_MEM_SIZE) + i] = buffer[i];
    }

    (*OF)++;

    return (*OF) - 1;
}

int checkAll(int logicalAddr, char* PT, struct TLB* tlb, char* PM, int* OF, int* pageFaults, int* TLBhits)
{
    unsigned char mask = 0xFF;
    unsigned char offset;
    unsigned char pageNum;
    bool TLBhit = false;
    int frame = 0;
    int value;
    int newFrame = 0;

    pageNum = (logicalAddr >> 8) & mask;

    offset = logicalAddr & mask;

    // check if in TLB
    int i = 0;
    for (i; i < TLB_SIZE; i++) {
        if (tlb->TLBpage[i] == pageNum) {
            frame = tlb->TLBframe[i];
            TLBhit = true;
            (*TLBhits)++;
        }
    }

    // check if in PageTable
    if (TLBhit == false) {
        if (PT[pageNum] != -1) {
        }

        // if not in either, read from disk
        else {
            newFrame = readDisk(pageNum, PM, OF);
            PT[pageNum] = newFrame;
            (*pageFaults)++;
        }
        frame = PT[pageNum];
        tlb->TLBpage[tlb->ind] = pageNum;
        tlb->TLBframe[tlb->ind] = PT[pageNum];
        tlb->ind = (tlb->ind + 1) % TLB_SIZE;
    }

    int index = ((unsigned char)frame * PHYS_MEM_SIZE) + offset;
    value = *(PM + index);
    
    printIndex = index;
    printValue = value;
    return 0;
}


int main(int argc, char* argv[]) {
    int val;
    FILE* fd;
    int openFrame = 0;
    int pageFaults = 0;
    int TLBhits = 0;
    int inputCount = 0;
    float pageFaultRate;
    float TLBHitRate;
    unsigned char PageTable[PAGE_SIZE];
    struct TLB tlb;
    char PhyMem[PHYS_MEM_SIZE][PHYS_MEM_SIZE];

    memset(PageTable, -1, sizeof(PageTable));
    memset(tlb.TLBpage, -1, sizeof(tlb.TLBpage));
    memset(tlb.TLBframe, -1, sizeof(tlb.TLBframe));
    tlb.ind = 0;

    if (argc < 2) {
        printf("ERROR: Not enough arguments");
        exit(0);
    }

    fd = fopen(argv[1], "r");
    if (fd == NULL) {
        printf("ERROR: File failed to open\n");
        exit(0);
    }

    FILE* fp1 = fopen("out1.txt", "wt");
    FILE* fp2 = fopen("out2.txt", "wt");
    FILE* fp3 = fopen("out3.txt", "wt");
    while (fscanf(fd, "%d", &val) == 1) {
        checkAll(val, PageTable, &tlb, (char*)PhyMem, &openFrame, &pageFaults, &TLBhits);
        inputCount++;
        
        fprintf(fp1, "Logical address: %d\n", val);
        fprintf(fp2, "Physical address: %d\n", printIndex);
        fprintf(fp3, "Value: %d\n", printValue);
    }
    
    fclose(fp1);
    fclose(fp2);
    fclose(fp3);

    pageFaultRate = (float)pageFaults / (float)inputCount;
    TLBHitRate = (float)TLBhits / (float)inputCount;
    
    printf("Page Fault Rate = %.4f", pageFaultRate);
    printf("\n");
    printf("TLB hit rate = %.4f", TLBHitRate);
    
    return 0;
}


