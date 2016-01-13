#include "myfs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Parameters */
FILE *fptr = NULL;
myfs_superblock_t superblock_info;

/* Function Prototypes */
void load_superblock(void);
void write_superblock(void);
void load_inode(void);
void write_inode(void);
void load_block(void);
void write_block(void);

/* Basic File System Function */
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

int myfs_open(const char *filesystem_name){
    if (fptr)
        myfs_close();
    if ((fptr = fopen(filesystem_name, "rb+"))) {
        load_superblock();
        return SUCCESS;
    }
    return FAILURE;
}

int myfs_close(){
    if (fptr) {
        write_superblock();
        fclose(fptr);
        fptr = NULL;
    }
    return SUCCESS;
}











/* Helping Functions */

void load_superblock(void) {
    fseek(fptr, 0, SEEK_SET);
    fread(&superblock_info, sizeof(myfs_superblock_t), 1, fptr);
    load_inode();
}

void write_superblock(void) {
    fseek(fptr, 0, SEEK_SET);
    fwrite(&superblock_info, sizeof(myfs_superblock_t), 1, fptr);
    write_inode();
}
