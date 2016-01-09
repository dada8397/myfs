#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

typedef int                     BOOL;
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

void display_main_menu(void) {
    system("clear");
    puts("****************    My FS Test Program    ****************");
    puts("*                                                        *");
    puts("* Before using APIs to manipulate file,                  *");
    puts("*   you have to open an existed filesystem.              *");
    puts("*                                                        *");
    printf("* Loaded Filesystem: %-35s *\n", (loaded_fs_name[0] == '\0') ? "(NIL)" : loaded_fs_name);
    printf("* Loaded File Path: %-36s *\n", (loaded_file_name[0] == '\0') ? "(NIL)" : loaded_file_name);
    puts("**********************************************************");
    puts("");
    puts("1. Create a new filesystem.");
    puts("2. Destroy an existed filesystem.");
    puts("3. Open an existed filesystem.");
    if (fs_loaded) {
        puts("4. Close loaded filesystem.");
        puts("");
        puts("5. Create a file.");
        puts("6. Delete a file.");
        printf("7. %s a file.\n", (file_loaded) ? "Close" : "Open");
        if (file_loaded) {
            puts("8. Write a file.");
            puts("9. Read a file.");
        }
        puts("");
        puts("10. Create a directory.");
        puts("11. Delete a directory.");
    }

    puts("");
    puts("99. Exit");
}

int read_options(void) {
    int result = 0;

    printf("Command options: ");
    scanf("%d%*c", &result);

    return result;
}

void process_task(int task) {
    int num;
    char str[512];

    puts("");

    switch(task) {
        case 1: { // Create filesystem
            printf("Please input filesystem name: ");
            fgets(str, sizeof(str), stdin);
            str[strcspn(str, "\n")] = '\0';
            printf("Please assign max size for this filesystem (bytes): ");
            scanf("%d%*c", &num);

            if (myfs_create(str, num) == FS_FAILURE)
                puts("\nFail to create filesystem.");
            else
                puts("\nFilesystem created.");
            break;
        }
        case 2: { // Destroy filesystem
            printf("Please input filesystem name: ");
            fgets(str, sizeof(str), stdin);
            str[strcspn(str, "\n")] = '\0';

            if (myfs_destroy(str) == FS_FAILURE)
                puts("\nFail to destroy filesystem.");
            else
                puts("\nFilesystem destroyed.");
            break;
        }
        case 3: { // Open filesystem
            printf("Please input filesystem name: ");
            fgets(str, sizeof(str), stdin);
            str[strcspn(str, "\n")] = '\0';

            if (myfs_open(str) == FS_FAILURE) {
                puts("\nFail to open filesystem.");
            } else {
                fs_loaded = TRUE;
                strcpy(loaded_fs_name, str);
                puts("\nFilesystem opened.");
            }
            break;
        }
        case 4: { // Close filesystem
            if (myfs_close() == FS_FAILURE) {
                puts("Fail to close filesystem.");
            } else {
                fs_loaded = FALSE;
                loaded_fs_name[0] = '\0';
                file_loaded = FALSE;
                loaded_file_name[0] = '\0';
                puts("Filesystem closed.");
            }
            break;
        }
        case 5: { // Create file
            printf("Please input abosulte path: ");
            fgets(str, sizeof(str), stdin);
            str[strcspn(str, "\n")] = '\0';

            if (myfs_file_create(str) == FS_FAILURE)
                puts("\nFail to create file.");
            else
                puts("\nAn empty file created.");
            break;
        }
        case 6: { // Delete file
            printf("Please input abosulte path: ");
            fgets(str, sizeof(str), stdin);
            str[strcspn(str, "\n")] = '\0';

            if (myfs_file_delete(str) == FS_FAILURE) {
                puts("\nFail to delete file.");
            } else {
                if (!strcmp(str, loaded_file_name)) {
                    loaded_file_name[0] = '\0';
                    file_loaded = FALSE;
                }
                puts("\nFile deleted.");
            }
            break;
        }
        case 7: { // Open file
            if (file_loaded) {
                file_loaded = FALSE;
                loaded_file_name[0] = '\0';
                myfs_file_close(loaded_fd);
                break;
            }

            printf("Please input abosulte path: ");
            fgets(str, sizeof(str), stdin);
            str[strcspn(str, "\n")] = '\0';

            if ((loaded_fd = myfs_file_open(str)) == FS_FAILURE) {
                puts("\nFail to open file.");
            } else {
                file_loaded = TRUE;
                strcpy(loaded_file_name, str);
                puts("\nFile opened.");
            }
            break;
        }
        case 8: { // Write file
            int written_bytes = 0;

            puts("Enter 1 if you want to write an external file to filesystem.");
            puts("Otherwise, please enter 2.\n");

            printf("Please enter your choice: ");
            scanf("%d%*c", &num);

            if (num == 1) {
                printf("\nInput filename which you wanted to save: ");
                fgets(str, sizeof(str), stdin);
                str[strcspn(str, "\n")] = '\0';

                FILE *fp = fopen(str, "rb");
                if (fp) {
                    fseek(fp, 0, SEEK_END);
                    int filesize = ftell(fp);
                    fseek(fp, 0, SEEK_SET);
                    char *buffer = (char *) malloc(filesize);
                    fread(buffer, 1, filesize, fp);
                    fclose(fp);

                    written_bytes = myfs_file_write(loaded_fd, buffer, filesize);
                } else {
                    puts("Fail to open external file.");
                }
            } else {
                printf("\nInput text which you wanted to save: ");
                fgets(str, sizeof(str), stdin);
                str[strcspn(str, "\n")] = '\0';

                written_bytes = myfs_file_write(loaded_fd, str, strlen(str));
            }

            printf("Written %d bytes to filesystem.\n", written_bytes);

            break;
        }
        case 9: { // Read file
            int read_bytes = 0;
            int want_bytes = 0;

            printf("Please input size which you want to read (bytes): ");
            scanf("%d%*c", &want_bytes);

            char *buffer = (char *) malloc(want_bytes);

            puts("\nEnter 1 if you want to output filesystem's content to an external file.");
            puts("Otherwise, please enter 2.\n");

            printf("Please enter your choice: ");
            scanf("%d%*c", &num);

            if (num == 1) {
                printf("\nInput filename which you wanted to save: ");
                fgets(str, sizeof(str), stdin);
                str[strcspn(str, "\n")] = '\0';

                FILE *fp = fopen(str, "wb");
                if (fp) {
                    read_bytes = myfs_file_read(loaded_fd, buffer, want_bytes);
                    fwrite(buffer, 1, read_bytes, fp);
                    fclose(fp);
                } else {
                    puts("Fail to open external file.");
                }
            } else {
                read_bytes = myfs_file_read(loaded_fd, buffer, want_bytes);
            }

            printf("Want to read %d bytes from filesystem.\nRead %d bytes from filesystem actually.\n", want_bytes, read_bytes);
            if (num == 1) {
                puts("Content saved to external file.");
            } else {
                puts("------- CONTENT start ------");
                for (int i = 0; i < read_bytes; ++i)
                    putchar(buffer[i]);
                puts("\n------- CONTENT end   ------");
            }

            break;
        }
        case 10: { // Create directory
            printf("Please input abosulte path: ");
            fgets(str, sizeof(str), stdin);
            str[strcspn(str, "\n")] = '\0';

            if (myfs_mkdir(str) == FS_FAILURE)
                puts("\nFail to create directory.");
            else
                puts("\nDirectory created.");
            break;
        }
        case 11: { // Remove directory
            printf("Please input abosulte path: ");
            fgets(str, sizeof(str), stdin);
            str[strcspn(str, "\n")] = '\0';

            if (myfs_rmdir(str) == FS_FAILURE)
                puts("\nFail to remove directory.");
            else
                puts("\nDirectory removed.");
            break;
        }
    }

    printf("Press enter to continue...");
    fgets(str, sizeof(str), stdin);
    str[strcspn(str, "\n")] = '\0';
}

int main(int argc, char *argv[]) {
    fs_loaded = FALSE;
    file_loaded = FALSE;

    load_apis();

    do {
        display_main_menu();

        int task = read_options();
        if (task == 99)
            break;

        process_task(task);
    } while(TRUE);

    return 0;
}
