#ifndef VCB_H
#define VCB_H

// think this is needed to track the file allocation table (FAT)
#define FAT_EOF 0xFFFFFFFF// End of file marker in FAT
#define FAT_FREE 0x0000000000000000 // Free block marker in FAT

#include <time.h>
#include <sys/types.h>

//not sure if these defines are needed
#define FS_SIGNATURE 0xCAFEBABE  // Unique signature for our file system
#define MAX_FILENAME_LENGTH 255
#define BLOCK_SIZE 4096          // 4KB blocks

typedef struct VolumeControlBlock {
    unsigned int numberOfBlocks; // Number of blocks within the FS
    unsigned int blockSize; // Size of the block
    unsigned int rootLocation; // Location of the root within the volume
    unsigned long signature; // How to determine if this blob is our blob, i.e will i need to initialize

    // ** FAT TABLE **
    unsigned int FATstart; // start of the FAT
    int *fatPtr; // location of FAT
    unsigned int locFreeBlock; // tracks start of the first free block within the FAT
    unsigned int fatTotalSize;

} VolumeControlBlock;
#endif