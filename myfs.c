#include "myfs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Parameters */
FILE *fptr = NULL;

/* Function Prototypes */

int myfs_create(const char *filesystem_name, int max_size){
    myfs_superblock_t superblock = {0};
    myfs_inode_t inode[max_size / MAX_BLOCK_SIZE] = {0};

    superblock.max_size = max_size;

    superblock.inode_total = max_size / MAX_BLOCK_SIZE;
    superblock.inode_bitmap_count = superblock.inode_total / (sizeof(uint) * 8);
    superblock.block_total = max_size / MAX_BLOCK_SIZE;
    superblock.block_bitmap_count = superblock.block_total / (sizeof(uint) * 8);

    superblock.inode_bitmap_offset = sizeof(myfs_superblock_t);
    superblock.block_bitmap_offset = superblock.inode_bitmap_offset
                                   + superblock.inode_bitmap_count;
    superblock.inode_offset = superblock.block_bitmap_offset
                            + superblock.block_bitmap_count;
    superblock.block_offset = superblock.inode_offset + superblock.inode_total;

    uint inode_bitmap[superblock.inode_bitmap_count] = {0};
    uint block_bitmap[superblock.block_bitmap_count] = {0};

    FILE *new_fs_file = fopen(filesystem_name, "wb");
    if(new_fs_file){
        fwrite(&superblock, sizeof(myfs_superblock_t), 1, new_fs_file);
        fwrite(&inode_bitmap, sizeof(uint), superblock.inode_bitmap_count, new_fs_file);
        fwrite(&block_bitmap, sizeof(uint), superblock.block_bitmap_count, new_fs_file);
        fwrite(&inode, sizeof(myfs_inode_t), max_size / MAX_BLOCK_SIZE, new_fs_file);
        return SUCCESS;
    }

    return FAILURE;
}
