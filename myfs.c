#include "myfs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Parameters */
FILE *fptr = NULL;
myfs_superblock_t superblock_info;
myfs_inode_t *root = NULL;
uint block_buf[MAX_BLOCK_SIZE / sizeof(uint)];

/* Function Prototypes */
void load_superblock(void);
void write_superblock(void);
void load_inode(void);
void write_inode(void);
void load_block(void);
void write_block(void);

/* Basic File System Function */
int myfs_create(const char *filesystem_name, int max_size){
    // uint 32 bits = 4 bytes
    uint inode_bitmap_count = max_size / MAX_BLOCK_SIZE / (sizeof(uint) * 8);
    uint block_bitmap_count = max_size / MAX_BLOCK_SIZE / (sizeof(uint) * 8);
    uint inode_total = max_size / MAX_BLOCK_SIZE;
    uint block_total = max_size / MAX_BLOCK_SIZE;

    myfs_superblock_t superblock = {
        .max_size = (uint) max_size,
        .unused_size = (uint) max_size,

        .inode_bitmap_offset = sizeof(myfs_superblock_t),
        .inode_bitmap_count = inode_bitmap_count,

        .block_bitmap_offset = (uint) (sizeof(myfs_superblock_t) + sizeof(uint) * inode_bitmap_count),
        .block_bitmap_count = block_bitmap_count,

        .inode_offset = (uint) (sizeof(myfs_superblock_t) + sizeof(uint) * inode_bitmap_count + sizeof(uint) * block_bitmap_count),
        .inode_total = inode_total,
        .inode_unused = inode_total,

        .block_offset = (uint) (sizeof(myfs_superblock_t) + sizeof(uint) * inode_bitmap_count + sizeof(uint) * block_bitmap_count + sizeof(myfs_inode_t) * inode_total),
        .block_total = block_total,
        .block_unused = block_total
    };

    uint *inode_bitmap = (uint*)calloc(inode_bitmap_count, sizeof(uint));
    uint *block_bitmap = (uint*)calloc(block_bitmap_count, sizeof(uint));
    myfs_inode_t *inode = (myfs_inode_t*) calloc(inode_total, sizeof(myfs_inode_t));

    FILE *new_fs_file = fopen(filesystem_name, "wb");
    if(new_fs_file){
        fwrite(&superblock, sizeof(myfs_superblock_t), 1, new_fs_file);
        fwrite(inode_bitmap, sizeof(uint), inode_bitmap_count, new_fs_file);
        fwrite(block_bitmap, sizeof(uint), block_bitmap_count, new_fs_file);
        fwrite(inode, sizeof(myfs_inode_t), inode_total, new_fs_file);
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
        fseek(fptr, 0, SEEK_SET);
        fread(&superblock_info, sizeof(myfs_superblock_t), 1, fptr);
        // Load Inode
        fseek(fptr, superblock_info.inode_offset, SEEK_SET);
        fread(root, sizeof(myfs_inode_t), 1, fptr);
        return SUCCESS;
    }
    return FAILURE;
}

int myfs_close(){
    if (fptr) {
        fseek(fptr, 0, SEEK_SET);
        fwrite(&superblock_info, sizeof(myfs_superblock_t), 1, fptr);
        // Write Inode

        fclose(fptr);
        fptr = NULL;
    }
    return SUCCESS;
}

int myfs_file_open(const char *filename);
int myfs_file_close(int fd);
int myfs_file_create(const char *filename);
int myfs_file_delete(const char *filename);
int myfs_file_read(int fd, char *buffer, int count);
int myfs_file_write(int fd, char *buffer, int count);
