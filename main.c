#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <dirent.h>
#include <ncurses.h>

typedef int                     BOOL;
typedef unsigned int            uint;

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
char command;

/* Functions Prototypes */
void load_apis(void);
void show_infomation(void);
void show_filesystems(void);
void show_main_command(void);
void show_file_command(void);

int main(int argc, char const *argv[]) {
    fs_loaded = FALSE;
    file_loaded = FALSE;
    load_apis();
    show_infomation();
    return 0;
}

void load_apis(void){
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

void show_infomation(void){
    printf("====================Welcome to virtual disk management!====================\n");
    printf("This is your filesystem below:\n");
    show_filesystems();
    show_main_command();
    printf("\n");
}

void show_filesystems(void){
    DIR *dir;
    struct dirent *ent;
    if((dir = opendir("./myfs/")) != NULL) {
        while((ent = readdir(dir)) != NULL) {
            if(strcmp(ent->d_name, ".") || strcmp(ent->d_name, "..")){
            }else{
                printf("%s\n", ent->d_name);
            }
        }
        closedir(dir);
    }else{
        perror("");
    }
}

void show_main_command(void){
    printf("There are somthing you can do:\n");
    printf("C : \e[4mC\e[mreate a filesystem.\n");
    printf("L : \e[4mL\e[moad a filesystem.\n");
    printf("D : \e[4mD\e[melete a filesystem.\n");
    printf("Please choose a command:");
    scanf("%c%*c", &command);
    switch(command) {
        case 'C':{
            printf("Please input the filesystem name:");
            char fs_name[512];
            scanf("%s%*c", fs_name);
            char fs_path[512] = "./myfs/";
            strcat(fs_path, fs_name);
            FILE *fptr;
            fptr = fopen(fs_path, "rb+");
            if(fptr != NULL){
                printf("Filesystem exist!\n");
                fclose(fptr);
                show_main_command();
                break;
            }else{
                printf("Please input the filesystem size(bytes):");
                int fs_size;
                scanf("%d%*c", &fs_size);
                if(myfs_create(fs_path, fs_size)){
                    printf("Create filesystem successfully!\n");
                    show_main_command();
                    break;
                }else{
                    printf("Create filesystem failed!\n");
                    show_main_command();
                    break;
                }
            }
            break;
        }
        case 'L':{
            printf("Please input the filesystem name:");
            char fs_name[512];
            scanf("%s%*c", fs_name);
            char fs_path[512] = "./myfs/";
            strcat(fs_path, fs_name);
            FILE *fptr;
            fptr = fopen(fs_path, "rb+");
            if(fptr != NULL){
                if(myfs_open(fs_path)){
                    printf("Load filesystem successfully!\n");
                    show_file_command();
                    break;
                }else{
                    printf("Load filesystem failed!\n");
                    show_main_command();
                    break;
                }
            }else{
                printf("Filesystem doesn't exist!\n");
                show_main_command();
                break;
            }
            break;
        }
        case 'D':{
            printf("Please input the filesystem name:");
            char fs_name[512];
            scanf("%s%*c", fs_name);
            char fs_path[512] = "./myfs/";
            strcat(fs_path, fs_name);
            FILE *fptr;
            fptr = fopen(fs_path, "rb+");
            if(fptr != NULL){
                if(myfs_destroy(fs_path)){
                    printf("Delete filesystem successfully!\n");
                    show_main_command();
                    break;
                }else{
                    printf("Delete filesystem failed!\n");
                    show_main_command();
                    break;
                }
            }else{
                printf("Filesystem doesn't exist!\n");
                show_main_command();
                break;
            }
            break;
        }
        default:{
            printf("\nCommand %c not found!\n", command);
            show_main_command();
            break;
        }
    }
}
