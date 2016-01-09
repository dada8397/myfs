/*
 * Header file for My File System
 *
 * File System Layout
 * ---------------------------------
 * |    File System Information    |
 * |    (file_system_t)            |
 * ---------------------------------
 * |    File Entry                 |
 * |    (file_entry_t)             |
 * ---------------------------------
 * |    File Content               |
 * |    (Binary data)              |
 * ---------------------------------
 */

#ifndef MY_FILE_SYSTEM_H
#define MY_FILE_SYSTEM_H

#define _GNU_SOURCE

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <libgen.h>

/* Common constants and types */
typedef int                     BOOL;
#define TRUE                    (1)
#define FALSE                   (0)

#define BUFFER_SIZE             (8192)
#define ENTRY_EOF               (0x1b)

#define MAX_FILE_NAME           (128)
#define MAX_FILE_COUNT          (1024)
#define MAX_FILE_PATH           (1024)

#define FS_SUCCESS              (0)
#define FS_FAILURE              (-1)
#define FS_READ_WRITE_FAILURE   (0)

/* Type of file entry */
typedef enum {
    TYPE_DIRECTORY = 0x0001,
    TYPE_FILE      = 0x0002
} file_attribute_t;

/* Required information of file system */
typedef struct file_system {
    int max_size;
    int used_size;

    int free_offset;
    int free_fd;
} file_system_t;

/* Required information of a file */
typedef struct file_entry {
    struct file_entry **children;
    int children_count;

    int fd;
    char name[MAX_FILE_NAME];
    int size;
    file_attribute_t attr;

    int raw_offset;
} file_entry_t;

/****************************************/

/* My File system APIs */
/*
 * Create a new filesystem.
 * If the specific filesystem has existed, all information and data in that will be erased.
 * Return: Return zero on success, or -1 if an error is occurred.
 */
int myfs_create(const char *filesystem_name, int max_size);
/*
 * Remove an existed filesystem.
 * Return: Return zero on success, or -1 if an error is occurred.
 */
int myfs_destroy(const char *filesystem_name);

/*
 * Open an existed filesystem.
 * Return: Return zero on success, or -1 if an error is occurred.
 */
int myfs_open(const char *filesystem_name);
/*
 * Close a filesystem safely.
 * Return: Return zero on success, or -1 if an error is occurred.
 */
int myfs_close(void);

/*
 * Open an existed file.
 * Return: Return a file descriptor, or -1 if an error is occurred.
 */
int myfs_file_open(const char *filename);
/*
 * Close a file descriptor.
 * Return: Return zero on success, or -1 if an error is occurred.
 */
int myfs_file_close(int fd);

/*
 * Create a new file.
 * If the specific file has existed, this method will return an error.
 * Return: Return zero on success, or -1 if an error is occurred.
 */
int myfs_file_create(const char *filename);
/*
 * Delete an existed file.
 * Return: Return zero on success, or -1 if an error is occurred.
 */
int myfs_file_delete(const char *filename);

/*
 * Read content from an opened file.
 * If `count` is larger than filesize, this method will read filesize bytes only.
 * Return: Return the number of bytes read from file.
 */
int myfs_file_read(int fd, char *buffer, int count);
/*
 * Write content to an opened file.
 * Return: Return the number of bytes written to file.
 */
int myfs_file_write(int fd, char *buffer, int count);

/*
 * Create a directory.
 * If the specific directory has existed, this method will return an error.
 * Return: Return zero on success, or -1 if an error is occurred.
 */
int myfs_mkdir(const char *filename);
/*
 * Remove an existed directory.
 * Return: Return zero on success, or -1 if an error is occurred.
 */
int myfs_rmdir(const char *filename);

#endif
