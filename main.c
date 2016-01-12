#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

typedef int                     BOOL;
typedef unsigned int            uint;
#define TRUE                    (1)
#define FALSE                   (0)

#define FS_SUCCESS              (0)
#define FS_FAILURE              (-1)

int (*myfs_create)(const char *filesystem_name, int max_size);
int (*myfs_destroy)(const char *filesystem_name);
int (*myfs_open)(const char *filesystem_name);
int (*myfs_close)(void);
int (*myfs_file_open)(const char *filename);
int (*myfs_file_close)(int fd);
int (*myfs_file_create)(const char *filename);
int (*myfs_file_delete)(const char *filename);
int (*myfs_file_read)(int fd, char *buffer, int count);
int (*myfs_file_write)(int fd, char *buffer, int count);
int (*myfs_mkdir)(const char *filename);
int (*myfs_rmdir)(const char *filename);

BOOL fs_loaded;
BOOL file_loaded;

char loaded_fs_name[512];
char loaded_file_name[512];

int loaded_fd;

void load_apis(void) {
    void* handle = dlopen("./myfs.so", RTLD_LAZY);

    myfs_create = (int (*)(const char*, int)) dlsym(handle, "myfs_create");
    myfs_destroy = (int (*)(const char*)) dlsym(handle, "myfs_destroy");
    myfs_open = (int (*)(const char*)) dlsym(handle, "myfs_open");
    myfs_close = (int (*)(void)) dlsym(handle, "myfs_close");
    myfs_file_open = (int (*)(const char*)) dlsym(handle, "myfs_file_open");
    myfs_file_close = (int (*)(int)) dlsym(handle, "myfs_file_close");
    myfs_file_create = (int (*)(const char*)) dlsym(handle, "myfs_file_create");
    myfs_file_delete = (int (*)(const char*)) dlsym(handle, "myfs_file_delete");
    myfs_file_read = (int (*)(int, char*, int)) dlsym(handle, "myfs_file_read");
    myfs_file_write = (int (*)(int, char*, int)) dlsym(handle, "myfs_file_write");
    myfs_mkdir = (int (*)(const char*)) dlsym(handle, "myfs_mkdir");
    myfs_rmdir = (int (*)(const char*)) dlsym(handle, "myfs_rmdir");
}

int main(int argc, char const *argv[]) {
    fs_loaded = FALSE;
    file_loaded = FALSE;
    load_apis();
    myfs_create("myfs", 1048576);
    return 0;
}
