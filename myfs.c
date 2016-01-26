#include "myfs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <libgen.h>
#include <sys/stat.h>

/* Parameters */
FILE *fptr = NULL;
myfs_superblock_t superblock_info;
myfs_file_t current;

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

    myfs_superblock_t superblock = {
        .max_size = (uint) max_size,
        .unused_size = (uint) max_size,
        .file_count = 0,
        .file_offset = (uint) sizeof(myfs_superblock_t)
    };

    myfs_file_t *file = (myfs_file_t*) calloc(MAX_FILE_AMOUNT, sizeof(myfs_file_t));

    FILE *new_fs_file = fopen(filesystem_name, "wb");
    if(new_fs_file){
        fwrite(&superblock, sizeof(myfs_superblock_t), 1, new_fs_file);
        fwrite(file, sizeof(myfs_file_t), MAX_FILE_AMOUNT, new_fs_file);
        fclose(new_fs_file);
        return SUCCESS;
    }

    return FAILURE;
}

int myfs_destroy(const char *filesystem_name){
    return (remove(filesystem_name) == 0) ? SUCCESS : FAILURE;
}

int myfs_open(const char *filesystem_name){
    if(fptr)
        myfs_close();
    if((fptr = fopen(filesystem_name, "rb+"))){
        fseek(fptr, 0, SEEK_SET);
        fread(&superblock_info, sizeof(myfs_superblock_t), 1, fptr);
        return SUCCESS;
    }
    return FAILURE;
}

int myfs_close(){
    if(fptr){
        fseek(fptr, 0, SEEK_SET);
        fwrite(&superblock_info, sizeof(myfs_superblock_t), 1, fptr);
        fclose(fptr);
        fptr = NULL;
    }
    return SUCCESS;
}

int myfs_file_open(const char *filename){
    uint file_amount = superblock_info.file_amount;
    for(uint i=0; i<file_amount; i++){
        fseek(fptr, i * sizeof(myfs_file_t) + superblock_info.file_offset, SEEK_SET);
        fread(&current, sizeof(myfs_file_t), 1, fptr);
        if(!strcmp(current.name, filename)){
            return SUCCESS;
        }
    }
    return FAILURE;
}
int myfs_file_close(int fd);
int myfs_file_create(const char *filename){
    // Count real count
    char *name = basename(strdupa(filename));
    myfs_file_t new_file = {0};
    new_file.index = (superblock_info.file_amount ++);
    superblock_info.file_count ++;
    new_file.offset = new_file.index * sizeof(myfs_file_t) + superblock_info.file_offset;
    new_file.mode = FILE_EXIST;
    strcpy(new_file.name, name);
    fseek(fptr, 0, SEEK_SET);
    fwrite(&superblock_info, sizeof(myfs_superblock_t), 1, fptr);
    fseek(fptr, new_file.index * sizeof(myfs_file_t) + superblock_info.file_offset, SEEK_SET);
    fwrite(&new_file, sizeof(myfs_file_t), 1, fptr);
    return SUCCESS;
}
int myfs_file_delete(const char *filename){
    uint file_amount = superblock_info.file_amount;
    for(uint i=0; i<file_amount; i++){
        fseek(fptr, i * sizeof(myfs_file_t) + superblock_info.file_offset, SEEK_SET);
        fread(&current, sizeof(myfs_file_t), 1, fptr);
        if(!strcmp(current.name, filename)){
            current.mode = FILE_DELETED;
            fseek(fptr, current.index * sizeof(myfs_file_t) + superblock_info.file_offset, SEEK_SET);
            fwrite(&current, sizeof(myfs_file_t), 1, fptr);
            superblock_info.file_count --;
            fseek(fptr, 0, SEEK_SET);
            fwrite(&superblock_info, sizeof(myfs_superblock_t), 1, fptr);
            return SUCCESS;
        }
    }
    return FAILURE;
}
int myfs_file_read(int fd, char *buffer, int count){
    printf("File %s :\n", current.name);
    printf("%s\n", current.content);
    return SUCCESS;
}
int myfs_file_write(int fd, char *buffer, int count){
    strcpy(current.content, buffer);
    fseek(fptr, current.index * sizeof(myfs_file_t) + superblock_info.file_offset, SEEK_SET);
    fwrite(&current, sizeof(myfs_file_t), 1, fptr);
    return SUCCESS;
}
int myfs_list_files(void){
    uint file_count = superblock_info.file_count;
    uint file_amount = superblock_info.file_amount;
    printf("Total %d files:\n", file_count);
    if(file_count == 0){
        return SUCCESS;
    }
    for(uint i=0; i<file_amount; i++){
        fseek(fptr, i * sizeof(myfs_file_t) + superblock_info.file_offset, SEEK_SET);
        fread(&current, sizeof(myfs_file_t), 1, fptr);
        if(current.mode == FILE_EXIST){
            printf("%s\n", current.name);
        }
    }
    return SUCCESS;
}
