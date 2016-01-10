#ifndef MY_FILE_SYSTEM_H
#define MY_FILE_SYSTEM_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <libgen.h>

/* Type : File or Directory */
typedef enum{
    TYPE_DIRECTORY,
    TYPE_FILE
} file_attribute_t;



int myfs_create(const char *filesystem_name, int max_size);
int myfs_destroy(const char *filesystem_name);
int myfs_open(const char *filesystem_name);
int myfs_close(void);

int myfs_file_open(const char *filename);
int myfs_file_close(int fd);
int myfs_file_create(const char *filename);
int myfs_file_delete(const char *filename);
int myfs_file_read(int fd, char *buffer, int count);
int myfs_file_write(int fd, char *buffer, int count);

int myfs_mkdir(const char *filename);
int myfs_rmdir(const char *filename);

#endif
