#include "myfs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Parameters */

/* Function Prototypes */

/* Create File System Function */
int myfs_create(const char *filesystem_name, int max_size){
    myfs_superblock_t superblock = {0};
    myfs_inode_t *inode = (myfs_inode_t*) calloc((max_size / MAX_BLOCK_SIZE), sizeof(myfs_inode_t));

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

    uint *inode_bitmap = (uint*)calloc(superblock.inode_bitmap_count, sizeof(uint));
    uint *block_bitmap = (uint*)calloc(superblock.block_bitmap_count, sizeof(uint));

    FILE *new_fs_file = fopen(filesystem_name, "wb");
    if(new_fs_file){
        fwrite(&superblock, sizeof(myfs_superblock_t), 1, new_fs_file);
        fwrite(inode_bitmap, sizeof(uint), superblock.inode_bitmap_count, new_fs_file);
        fwrite(block_bitmap, sizeof(uint), superblock.block_bitmap_count, new_fs_file);
        fwrite(inode, sizeof(myfs_inode_t), superblock.inode_total, new_fs_file);
        fclose(new_fs_file);
        return SUCCESS;
    }

    return FAILURE;
}

int myfs_destroy(const char *filesystem_name){
    return (remove(filesystem_name) == 0) ? SUCCESS : FAILURE;
}
