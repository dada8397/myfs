#ifndef MY_FILE_SYSTEM_H
#define MY_FILE_SYSTEM_H

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <libgen.h>

int myfs_create(const char *filesystemname, int max_size);
int myfs_destroy(const char * filesystemname);

int myfs_file_open(const char *filename);
int myfs_file_close(int fd);
int myfs_file_create(const char * filename);
int myfs_file_delete(const char * filename);
int myfs_file_read(int fd, char *buf, int count);
int myfs_file_write(int fd, char *buf, int count);

#endif
