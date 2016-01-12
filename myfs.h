#ifndef MY_FILE_SYSTEM_H
#define MY_FILE_SYSTEM_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <libgen.h>

/** Constants and Types **/
typedef unsigned int            uint;
typedef int                     BOOL;

#define TRUE                    (1)
#define FALSE                   (0)

#define BUFFER_SIZE             (8192)
#define ENTRY_EOF               (0x1b)

// Superblock Constants
#define MAX_FILE_NAME           (255)
#define MAX_FILE_PATH           (4096)
#define MAX_FILE_AMOUNT         (1024)

// Inode Constants
#define INODE_BLOCK_N           (15)

// Block Constants
#define MAX_BLOCK_SIZE          (4096)

#define FAILURE                 (-1)
#define SUCCESS                 (0)

/* Type : File or Directory */
typedef enum{
    TYPE_DIRECTORY  = 0x0001,
    TYPE_FILE       = 0x0002
} file_attribute_t;

/* Superblock Structure */
typedef struct myfs_superblock {
    uint max_size;
    uint unused_size;

    uint inode_bitmap_offset;
    uint inode_bitmap_count;
    uint inode_offset;
    uint inode_total;
    uint inode_unused;

    uint block_bitmap_offset;
    uint block_bitmap_count;
    uint block_offset;
    uint block_total;
    uint block_unused;
} myfs_superblock_t;

/* Inode Structure */
typedef struct myfs_inode {
    uint index;
    uint mode;
    uint next;
    union {
        uint child;
        uint size;
    };
    uint block_count;
    uint block[INODE_BLOCK_N];
    char name[MAX_FILE_NAME];
} myfs_inode_t;

/* Block Structure */
typedef struct myfs_block {

} myfs_block_t;

/* File System Prototypes */
int myfs_create(const char *filesystem_name, int max_size);
int myfs_destroy(const char *filesystem_name);

/* File System Prototypes of Extra Functions */
int myfs_open(const char *filesystem_name);
int myfs_close(void);

/* File Prototypes */
int myfs_file_open(const char *filename);
int myfs_file_close(int fd);
int myfs_file_create(const char *filename);
int myfs_file_delete(const char *filename);
int myfs_file_read(int fd, char *buffer, int count);
int myfs_file_write(int fd, char *buffer, int count);

/* File Prototypes of Extra Functions*/
int myfs_mkdir(const char *filename);
int myfs_rmdir(const char *filename);

#endif
