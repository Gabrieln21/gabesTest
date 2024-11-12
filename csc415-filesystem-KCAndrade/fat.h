#ifndef FAT_H
#define FAT_H
#include <stdio.h>
#include "vcb.h"

//int FAT[TOTAL_BLOCKS];
int initializeFat(VolumeControlBlock *vcbPtr, u_int64_t blockSize, u_int64_t totalBlocks);
int allocateBlocks(VolumeControlBlock *vcbPtr, u_int64_t numBlocks);
int releaseBlocks (u_int64_t blockLocation, u_int64_t numOfBlocks);

#endif
