/**************************************************************
* Class::  CSC-415-0# Spring 2024
* Name::
* Student IDs::
* GitHub-Name::
* Group-Name::
* Project:: Basic File System
*
* File:: fsInit.c
*
* Description:: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fat.h"
#include "fsLow.h"
#include "mfs.h"
#include "vcb.h"

VolumeControlBlock *vcbPtr = NULL;
DirectoryEntry *rootDir = NULL;

struct DirectoryEntry* createDirectory(int numEntries, struct DirectoryEntry *parent, struct VolumeControlBlock *vcb);

/*
*** FUNCTION DESCRIPTION ***
* Init File System is the entry point into the creation of our FS
* Here we are initializing the VCB and the root dir
* @param unsigned long numberOfBlocks: is the number of blocks that the file system will be initialized with
* @param unsigned long blockSize: the size of each block within the FS
* @return returns is -1 if initFileSystem fails and 0 for a succesful initialization
* function ends on line 124
*/
int initFileSystem(u_int64_t numberOfBlocks, uint64_t blockSize) {
    
    printf("Initializing File System with %ld blocks with a block size of %ld\n",
           numberOfBlocks, blockSize);

    // Allocate and initialize VCB
    vcbPtr = malloc(blockSize);
    if (vcbPtr == NULL) {
        printf("Error: Unable to allocate memory for VCB\n");
        return -1;
    }
    //LBA read returns the number of blocks read, VCB only uses one block so LBAread should return 1
    LBAread(vcbPtr, 1, 0);
    // Format volume if signatures do not match
    if (vcbPtr->signature != FS_SIGNATURE) {
        printf("No valid file system found. Formatting volume...\n");

        // Initialize VCB
        vcbPtr->signature = FS_SIGNATURE;
        vcbPtr->blockSize = blockSize;
        vcbPtr->numberOfBlocks = numberOfBlocks;
        
        // ** REVIEW FAT FUNCTION **
        // Initialize FAT
        if (initializeFat(vcbPtr, blockSize, numberOfBlocks) < 0) {
            printf("Error: Failed to initialize FAT\n");
            free(vcbPtr);
            return -1;
        }

        // ** REVIEW INIT ROOT DIR CODE **  
        // Initialize root directory
        struct DirectoryEntry* rootDirStart = createDirectory(51, NULL, vcbPtr);
        if (rootDirStart < 0) {
            printf("Error: Failed to initialize root directory\n");
            free(vcbPtr);
            return -1;
        }
        vcbPtr->rootLocation = rootDirStart->firstBlockIndex;
        LBAwrite(vcbPtr, 1, 0);
    }
    

    return 0;
}
	

//Pass in null for parent to set up rootDirectory
struct DirectoryEntry* createDirectory(int numEntries, struct DirectoryEntry *parent, struct VolumeControlBlock *vcb) {
    printf("Entering createDirectory with numEntries: %d\n", numEntries); // Debug

    int bytesNeeded = numEntries * sizeof(DirectoryEntry);
    int blocksNeeded = (bytesNeeded + (vcb->blockSize - 1)) / vcb->blockSize;
    int actualBytes = blocksNeeded * vcb->blockSize;
    int actualEntries = actualBytes/sizeof(DirectoryEntry);

    printf("  bytesNeeded: %d, blocksNeeded: %d\n", bytesNeeded, blocksNeeded); // Debug

    // Allocate blocks for the directory
    DirectoryEntry * new = malloc(actualBytes);
    int dirLocation = allocateBlocks(vcb, blocksNeeded);

    if (dirLocation == -1) {
        perror("Error allocating blocks for directory");
        return NULL;
    }

    printf("  Allocated blocks for directory at location: %d\n", dirLocation); // Debug


    printf("  Allocated memory for directory entries at: %p\n", (void *)new); // Debug

    // Initialize directory entries to a known free state
    for (int i = 2; i < actualEntries; i++ ) {
        new[i].inUse = 0;
    }

    // Set up "." entry
    strcpy(new[0].filename, ".");
    new[0].firstBlockIndex = dirLocation;
    new[0].fileSize = actualEntries * sizeof(DirectoryEntry);
    new[0].fileType = 1;
    new[0].creationTime = time(NULL);
    new[0].lastModifiedTime = new[0].creationTime;
    new[0].lastAcessedTime = new[0].creationTime;
    new[0].inUse = 1; // Add this line

    // Set up ".." entry 
    if (parent == NULL) {
        // Root directory case
        strcpy(new[1].filename, "..");
        new[1].firstBlockIndex = new[0].firstBlockIndex; 
        new[1].fileSize = new[0].fileSize;             
        new[1].creationTime = new[0].creationTime;     // Copy creationTime
        new[1].lastModifiedTime = new[0].lastModifiedTime; // Copy lastModifiedTime
        new[1].lastAcessedTime = new[0].lastAcessedTime; 
        new[1].fileType = 1;                          // Set fileType to directory
        new[1].inUse = 1;                            // Add this line
    } else {
        // Regular directory case: ".." points to the parent
        strcpy(new[1].filename, "..");
        new[1].firstBlockIndex = parent->firstBlockIndex;
        new[1].fileSize = parent->fileSize;
        new[1].fileType = 1; // Directory type
        new[1].creationTime = time(NULL); // Set creation time
        new[1].lastModifiedTime = new[1].creationTime; // Set last modified time
        new[1].lastAcessedTime = new[1].creationTime; // Set last accessed time
        new[1].inUse = 1; // Mark as in use
    }

    
    
    int blocksWritten = LBAwrite(new, blocksNeeded, vcb->rootLocation);
    if (blocksWritten != blocksNeeded) {
        printf("Error: Directory blocks written (%d) do not match expected (%d)\n", blocksWritten, blocksNeeded);
        free(new);
        return NULL;
    }

    return new;
}

void exitFileSystem (){
    printf ("System exiting\n");
}
