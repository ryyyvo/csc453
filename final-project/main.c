#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libDisk.h"
#include "error.h"

#define BLOCKSIZE 256
#define NUM_BLOCKS 96  // 4 platters * 3 tracks * 8 sectors
#define NUM_PLATTERS 4
#define NUM_TRACKS 3
#define NUM_SECTORS 8

#define SUPERBLOCK 0
#define MIN_FILE_ID 2
#define MAX_FILE_ID 9

typedef struct {
    unsigned char data[BLOCKSIZE];
} Block;

Block disk[NUM_BLOCKS];
int step_counter = 0;

void initDisk() {
    memset(disk, 0, sizeof(disk));
    
    // initialize superblock
    disk[SUPERBLOCK].data[0] = 1;
    
    for (int i = 1; i < NUM_BLOCKS; i++) {
        disk[SUPERBLOCK].data[i] = 0;  // 0 = free
    }
    disk[SUPERBLOCK].data[SUPERBLOCK] = 1;
}

int writeDiskToFile(int stepNum) {
    char filename[100];
    sprintf(filename, "Release/step_%d.bin", stepNum);
    
    int fd = openDisk(filename, NUM_BLOCKS * BLOCKSIZE);
    if (fd < 0) {
        printf("Error: Could not create %s\n", filename);
        return -1;
    }
    
    // write blocks 
    for (int i = 0; i < NUM_BLOCKS; i++) {
        if (writeBlock(fd, i, disk[i].data) != 0) {
            printf("Error: Could not write block %d\n", i);
            closeDisk(fd);
            return -1;
        }
    }
    
    closeDisk(fd);
    return 0;
}

int findVerticalBlocks(int numBlocks, int *blocks) {
    int blocksFound = 0;
    
    for (int track = 0; track < NUM_TRACKS; track++) {
        for (int sector = 0; sector < NUM_SECTORS; sector++) {
            int cylinderBlocks[NUM_PLATTERS];
            int freeCylinder = 1;
            
            for (int platter = 0; platter < NUM_PLATTERS; platter++) {
                // calculate block index
                int blockIdx = sector + track * NUM_SECTORS + platter * (NUM_TRACKS * NUM_SECTORS);
                cylinderBlocks[platter] = blockIdx;
                
                // check if block is free
                if (disk[SUPERBLOCK].data[blockIdx] != 0) {
                    freeCylinder = 0;
                    break;
                }
            }
            
            // if cylinder free, use as many blocks as needed
            if (freeCylinder) {
                for (int p = 0; p < NUM_PLATTERS && blocksFound < numBlocks; p++) {
                    blocks[blocksFound++] = cylinderBlocks[p];
                }
                
                if (blocksFound >= numBlocks) {
                    return 0;
                }
            }
        }
    }
    
    return -1;
}

int createFile(int fileID, int numBlocks) {
    if (fileID < MIN_FILE_ID || fileID > MAX_FILE_ID) {
        printf("Error: Invalid file ID\n");
        return -1;
    }
    
    if (numBlocks < 1 || numBlocks > NUM_BLOCKS - 1) {
        printf("Error: Invalid number of blocks %d\n", numBlocks);
        return -1;
    }
    
    // check if file exists
    for (int i = 1; i < NUM_BLOCKS; i++) {
        if (disk[i].data[0] == fileID) {
            printf("Error: File %d already exists\n", fileID);
            return -1;
        }
    }
    
    // find free blocks
    int *blocks = malloc(numBlocks * sizeof(int));
    if (findVerticalBlocks(numBlocks, blocks) != 0) {
        printf("Error: Not enough free blocks for file %d\n", fileID);
        free(blocks);
        return -1;
    }
    
    int inodeBlock = blocks[0];
    disk[inodeBlock].data[0] = fileID;
    
    // store indices of data blocks in inode
    for (int i = 1; i < numBlocks; i++) {
        disk[inodeBlock].data[i] = blocks[i];
    }
    
    // mark inode block as used in superblock
    disk[SUPERBLOCK].data[inodeBlock] = 1;
    
    // initialize data blocks
    for (int i = 1; i < numBlocks; i++) {
        int blockIdx = blocks[i];
        disk[blockIdx].data[0] = fileID;
        disk[SUPERBLOCK].data[blockIdx] = 1;
    }
    
    printf("Created file %d with %d blocks\n", fileID, numBlocks);
    free(blocks);
    return 0;
}

int deleteFile(int fileID) {
    if (fileID < MIN_FILE_ID || fileID > MAX_FILE_ID) {
        printf("Error: Invalid file ID\n");
        return -1;
    }
    
    // find the inode block
    int inodeBlock = -1;
    for (int i = 1; i < NUM_BLOCKS; i++) {
        if (disk[i].data[0] == fileID) {
            inodeBlock = i;
            break;
        }
    }
    
    if (inodeBlock == -1) {
        printf("Error: File %d does not exist\n", fileID);
        return -1;
    }
    
    // read data block indices from inode and free them
    for (int i = 1; i < BLOCKSIZE; i++) {
        int blockIdx = (unsigned char)disk[inodeBlock].data[i];
        if (blockIdx == 0) {
            break;  // no more blocks
        }
        if (blockIdx < NUM_BLOCKS && disk[blockIdx].data[0] == fileID) {
            memset(disk[blockIdx].data, 0, BLOCKSIZE);
            disk[SUPERBLOCK].data[blockIdx] = 0;
        }
    }
    
    memset(disk[inodeBlock].data, 0, BLOCKSIZE);
    disk[SUPERBLOCK].data[inodeBlock] = 0;
    
    printf("Deleted file %d\n", fileID);
    return 0;
}

int main() {
    FILE *inputFile = fopen("input.txt", "r");
    if (!inputFile) {
        printf("Error: Could not open input.txt\n");
        return 1;
    }

    initDisk();
    
    writeDiskToFile(step_counter++);
    
    char command[20];
    int arg1, arg2;
    
    while (fscanf(inputFile, "%s", command) != EOF) {
        if (strcmp(command, "CREATE") == 0) {
            if (fscanf(inputFile, "%d %d", &arg1, &arg2) != 2) {
                printf("Error: CREATE requires 2 arguments\n");
                break;
            }
            createFile(arg1, arg2);
        } else if (strcmp(command, "DELETE") == 0) {
            if (fscanf(inputFile, "%d", &arg1) != 1) {
                printf("Error: DELETE requires 1 argument\n");
                break;
            }
            deleteFile(arg1);
        } else {
            printf("Error: Unknown command '%s'\n", command);
            break;
        }
        
        writeDiskToFile(step_counter++);
    }
    
    fclose(inputFile);
    
    return 0;
}