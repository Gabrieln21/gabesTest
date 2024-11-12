#include "mfs.h"
#include "fat.h"
#include "vcb.h"
#include "fsLow.h"
#include <stdio.h>
#include <stdlib.h>

    #define FAT_EOF 0xFFFFFFFF// End of file marker in FAT
    #define FAT_FREE 0x0000000000000000 // Free block marker in FAT
    int freeSpace_start = -1;
    int FREE_BLOCK; 
    int END_OF_CHAIN  = 0;

int initializeFat(VolumeControlBlock *vcbPtr, u_int64_t blockSize, u_int64_t totalBlocks) {
    // Calculate the number of bytes needed for the FAT and number of blocks required
    u_int64_t fatBytes = totalBlocks * sizeof(unsigned int);  // Allocate enough bytes for each FAT entry
    u_int64_t blocksNeeded = (fatBytes + blockSize - 1) / blockSize;  // Round up to the nearest block
    vcbPtr->fatTotalSize = blocksNeeded;
    vcbPtr->rootLocation = blocksNeeded + 1;

    // ** DEBUG **
    printf("======*DEBUG*=======\n");
    printf("Amount of blocks being allocated for FAT: %llu\n", blocksNeeded);

    // Allocate memory for FAT
    vcbPtr->fatPtr = malloc(fatBytes);
    if (vcbPtr->fatPtr == NULL) {
        return -1;  // Allocation failed
    }

    // Initialize FAT in memory
    unsigned int *fat = (unsigned int *)vcbPtr->fatPtr;

    // First mark all blocks as free
    for (uint64_t i = 0; i < totalBlocks; i++) {
        fat[i] = FAT_FREE;
    }

    // Mark system blocks as used (EOF means they're in use and not part of a chain)
    fat[0] = FAT_EOF;  // VCB block is used

    /*
    for (uint64_t i = 1; i <= blocksNeeded; i++) {
        fat[i] = FAT_EOF;  // FAT blocks are used
    }
    // Set first free block pointer to the first block after the FAT
    vcbPtr->locFreeBlock = blocksNeeded + 1;
    for (uint64_t i = 0; i < totalBlocks - 1; i++) {
            if (i != 0) {
                fat[i] = i + 1;  // Link each block to the next, skipping block 2
            }
    }
    */
    

    // Set up free block chain
    

    // Mark the last block as the end of the chain
    if (totalBlocks > 1 && totalBlocks - 1 != 2) {
        fat[totalBlocks - 1] = FAT_EOF;
    }

    // Set the start of the FAT in the Volume Control Block
    vcbPtr->FATstart = 1;  // Typically, the first block is used for VCB, so FAT starts at block 1
    vcbPtr->locFreeBlock = 1;  // Set the first free block to 0

    // Write the FAT to disk using LBAwrite
    int blocksWritten = LBAwrite(vcbPtr->fatPtr, blocksNeeded, vcbPtr->FATstart);
    printf("Amount of blocks written: %d\n", blocksWritten);
    if (blocksWritten != blocksNeeded) {
        printf("\nDifferent amount of blocks written than expected, expected blocks: %llu\n", blocksNeeded);
        printf("Amount of blocks actually written: %d\n", blocksWritten);
        return -1;
    }

    return 0;
}

int findFirstFreeBlock(VolumeControlBlock *vcbPtr, int startIndex) {
    unsigned int *fat = (unsigned int *)vcbPtr->fatPtr;
    for (int i = startIndex; i < vcbPtr->numberOfBlocks; i++) {
        printf("the value of fat[%d]: %d\n", i, fat[i]);
        if (fat[i] == FAT_FREE){
            return i;
        }
    }
    return -1; // No free block found
}

int allocateBlocks(VolumeControlBlock *vcbPtr, u_int64_t numBlocks) {
    unsigned int *fat = (unsigned int *)vcbPtr->fatPtr;
    int numBlocksCounter = 0;
    int firstFree = findFirstFreeBlock(vcbPtr, vcbPtr->locFreeBlock);

    if (firstFree == -1) {
        printf("Error: No free blocks available\n");
        return -1; 
    }

    int previousFree = firstFree;
    vcbPtr->locFreeBlock = firstFree + 1; // Start after the first block

    while (numBlocksCounter < numBlocks - 1) {
        int nextFree = findFirstFreeBlock(vcbPtr, vcbPtr->locFreeBlock);
        if (nextFree == -1) {
            printf("Error: Not enough free blocks\n");
            return -1; 
        }

        // Link the current block to the next one
        fat[previousFree] = nextFree;
        previousFree = nextFree;
        vcbPtr->locFreeBlock = nextFree + 1; // Update to look for the next block
        numBlocksCounter++;
    }

    // Mark the end of the chain
    
    
    fat[previousFree] = FAT_EOF;

    // Update locFreeBlock to the next free block
    vcbPtr->locFreeBlock = findFirstFreeBlock(vcbPtr, previousFree + 1);
    LBAwrite(vcbPtr->fatPtr, vcbPtr->fatTotalSize, vcbPtr->FATstart);

    return firstFree;
}


/* 
int releaseBlocks (u_int64_t blockLocation, u_int64_t numOfBlocks) {
    //we should verify if this a valid call (here or wherever its called)
    //to verify here woukd need to pass in vcb
  
    unsigned int currentBlock = blockLocation;
    unsigned int total = blockLocation + numOfBlocks;

    for (u_int64_t i = blockLocation; i <= total; i++) {
        FAT[i] = FAT_FREE; // Mark the current block as free
    }

    //TODO: 
    //remember to update VCB fat variables when calling this fuction

    //TODO:
    //wether in this fucntion or where we call this fucnction we need to set corrisponding directory entrys to inUse = 0.
    return 0; // Success
}
*/
