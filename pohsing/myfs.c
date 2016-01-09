#include "myfs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* File system Helper */
void load_filesystem(void);
void write_filesystem(void);
void load_file_table(void);
void write_file_table(void);
void free_file_table(void);

void serialize_file_table(file_entry_t *current);
BOOL deserialize_file_table(file_entry_t **current);

file_entry_t *find_directory(const char *filename);
file_entry_t *child_existed(file_entry_t *parent, const char *name);
file_entry_t **realloc_entry_pointer_array(file_entry_t **orig, int new_size);

void remove_child(file_entry_t *parent, const char *name);
void insert_child(file_entry_t *parent, file_entry_t new_entry);

void delete_data(int offset, int delete_size);
int insert_data(int offset, const char *buffer, int insert_size);
void update_file_raw_offset(file_entry_t *current, file_entry_t *skip, int threshold, int offset);

file_entry_t *valid_fd(int fd);

/****************************************/

FILE *fp = NULL;
file_system_t filesystem_info;
file_entry_t *root = NULL;
int opened_file_count;
file_entry_t **opened_file = NULL;

/****************************************/

int myfs_create(const char *filesystem_name, int max_size) {
    file_system_t fs = { 0 };
    file_entry_t fe[MAX_FILE_COUNT] = { 0 };

    fs.max_size = max_size;
    fs.free_offset = sizeof(file_system_t) + sizeof(fe);

    FILE *new_fp = fopen(filesystem_name, "wb");
    if (new_fp) {
        fwrite(&fs, sizeof(file_system_t), 1, new_fp);
        fwrite(&fe, sizeof(file_entry_t), MAX_FILE_COUNT, new_fp);
        fclose(new_fp);

        return FS_SUCCESS;
    }

    return FS_FAILURE;
}

int myfs_destroy(const char *filesystem_name) {
    return (remove(filesystem_name) == 0) ? FS_SUCCESS : FS_FAILURE;
}

int myfs_open(const char *filesystem_name) {
    if (fp)
        myfs_close();

    if ((fp = fopen(filesystem_name, "rb+"))) {
        load_filesystem();

        return FS_SUCCESS;
    }

    return FS_FAILURE;
}

int myfs_close(void) {
    if (fp) {
        write_filesystem();
        free_file_table();

        fclose(fp);
        fp = NULL;
    }

    return FS_SUCCESS;
}

int myfs_file_open(const char *filename) {
    char *directory = dirname(strdupa(filename));
    char *name = basename(strdupa(filename));

    file_entry_t *parent = find_directory(directory);
    if (!parent) {
        printf("Invalid Path: %s\n", filename);

        return FS_FAILURE;
    }

    file_entry_t *child = child_existed(parent, name);
    if (!child) {
        printf("Invalid Path: %s\n", filename);

        return FS_FAILURE;
    }

    if (!opened_file)
        opened_file = (file_entry_t**) malloc(sizeof(file_entry_t*));
    else
        opened_file = realloc_entry_pointer_array(opened_file, sizeof(file_entry_t*) * (opened_file_count + 1));

    opened_file[opened_file_count++] = child;
    return child->fd;
}

int myfs_file_close(int fd) {
    if (!valid_fd(fd))
        return FS_FAILURE;

    //int delete_index = 0;
    file_entry_t** new_opened_file = NULL;

    if (opened_file_count > 1) {
        new_opened_file = (file_entry_t**) malloc(sizeof(file_entry_t*) * (opened_file_count - 1));
        for (int i = 0, k = 0; i < opened_file_count; ++i) {
            if (fd != opened_file[i]->fd)
                new_opened_file[k++] = opened_file[i];
        }
    }

    free(opened_file);
    opened_file = new_opened_file;
    --opened_file_count;

    write_filesystem();
    return FS_SUCCESS;
}

int myfs_file_create(const char *filename) {
    char *directory = dirname(strdupa(filename));
    char *name = basename(strdupa(filename));

    file_entry_t *parent = find_directory(directory);
    if (!parent) {
        printf("Invald Path: %s\n", filename);
        return FS_FAILURE;
    }

    if (child_existed(parent, name)) {
        printf("File Existed: %s\n", filename);
        return FS_FAILURE;
    } else {
        printf("Creating File: %s\n", filename);

        file_entry_t new_entry = { 0 };
        new_entry.fd = filesystem_info.free_fd++;
        new_entry.attr = TYPE_FILE;
        new_entry.raw_offset = filesystem_info.free_offset;
        strcpy(new_entry.name, name);
        insert_child(parent, new_entry);
    }

    return FS_SUCCESS;
}

int myfs_file_delete(const char *filename) {
    char *directory = dirname(strdupa(filename));
    char *name = basename(strdupa(filename));

    file_entry_t *parent = find_directory(directory);
    if (!parent) {
        printf("Invalid Path: %s\n", filename);
        return FS_FAILURE;
    }

    file_entry_t *child = child_existed(parent, name);
    if (!child) {
        printf("Invalid Path: %s\n", filename);
        return FS_FAILURE;
    }

    if (valid_fd(child->fd)) {
        printf("File opened, can't delete it: %s\n", filename);
        return FS_FAILURE;
    }

    printf("Removing File: %s\n", filename);

    delete_data(child->raw_offset, child->size);
    update_file_raw_offset(root, child, child->raw_offset, 0 - child->size);
    filesystem_info.free_offset -= child->size;
    filesystem_info.used_size -= child->size;

    remove_child(parent, name);

    return FS_SUCCESS;
}

int myfs_file_read(int fd, char *buffer, int count) {
    file_entry_t *current = valid_fd(fd);
    int read_count = (count < current->size) ? count : current->size;

    fseek(fp, current->raw_offset, SEEK_SET);
    return fread(buffer, 1, read_count, fp);
}

int myfs_file_write(int fd, char *buffer, int count) {
    file_entry_t *current = valid_fd(fd);
    int written_bytes = 0;

    fseek(fp, current->raw_offset, SEEK_SET);
    if (current->raw_offset == filesystem_info.free_offset) {
        written_bytes = fwrite(buffer, 1, count, fp);
    } else {
        written_bytes = insert_data(current->raw_offset + current->size, buffer, count);
        update_file_raw_offset(root, current, current->raw_offset, written_bytes);
    }

    filesystem_info.free_offset += written_bytes;
    filesystem_info.used_size += written_bytes;
    current->size += written_bytes;

    return written_bytes;
}

int myfs_mkdir(const char *filename) {
    if (find_directory(filename)) {
        printf("Directory Existed\n");
        return FS_FAILURE;
    }

    char *directory = dirname(strdupa(filename));
    char *name = basename(strdupa(filename));

    file_entry_t *parent = find_directory(directory);
    if (!parent) {
        printf("Invalid Path: %s\n", filename);
        return FS_FAILURE;
    }

    if (child_existed(parent, name)) {
        printf("Directory Existed: %s\n", filename);
        return FS_FAILURE;
    } else {
        printf("Creating Directory: %s\n", filename);

        file_entry_t new_entry = { 0 };
        new_entry.attr = TYPE_DIRECTORY;
        strcpy(new_entry.name, name);
        insert_child(parent, new_entry);
    }

    return FS_SUCCESS;
}

int myfs_rmdir(const char *filename) {
    int result = FS_FAILURE;
    char *directory = dirname(strdupa(filename));
    char *name = basename(strdupa(filename));

    file_entry_t *parent = find_directory(strdupa(directory));
    if (!parent) {
        printf("Invalid Path: %s\n", filename);
        return FS_FAILURE;
    }

    file_entry_t *child = child_existed(parent, name);
    if (!child || child->attr != TYPE_DIRECTORY) {
        printf("Invalid Path: %s\n", filename);
        return FS_FAILURE;
    }

    printf("Removing Directory: %s\n", filename);

    for (int i = 0; i < child->children_count; ++i) {
        char full_path[MAX_FILE_PATH] = { 0 };

        sprintf(full_path, "%s/%s/%s", directory, name, child->children[i]->name);

        if (child->children[i]->attr == TYPE_DIRECTORY) {
            printf("Removing SubDirectory: %s\n", full_path);
            if ((result = myfs_rmdir(full_path)) != FS_SUCCESS)
                break;
        } else {
            printf("Removing SubFile: %s\n", full_path);
            if ((result = myfs_file_delete(full_path)) != FS_SUCCESS)
                break;
        }
    }

    if (result == FS_SUCCESS)
        remove_child(parent, name);

    return result;
}

/****************************************/

void load_filesystem(void) {
    fseek(fp, 0, SEEK_SET);
    fread(&filesystem_info, sizeof(file_system_t), 1, fp);
    load_file_table();
}

void write_filesystem(void) {
    fseek(fp, 0, SEEK_SET);
    fwrite(&filesystem_info, sizeof(file_system_t), 1, fp);
    write_file_table();
}

void load_file_table(void) {
    opened_file_count = 0;
    if (opened_file) {
        free(opened_file);
        opened_file = NULL;
    }

    fseek(fp, sizeof(file_system_t), SEEK_SET);
    deserialize_file_table(&root);
}

void write_file_table(void) {
    fseek(fp, sizeof(file_system_t), SEEK_SET);
    serialize_file_table(root);
}

void free_file_table(void) {
}

void serialize_file_table(file_entry_t *current) {
    if (!current)
        return;

    file_entry_t marker = { 0 };
    marker.name[0] = ENTRY_EOF;

    fwrite(current, sizeof(file_entry_t), 1, fp);
    for (int i = 0; i < current->children_count; ++i)
        serialize_file_table(current->children[i]);
    fwrite(&marker, sizeof(file_entry_t), 1, fp);
}

BOOL deserialize_file_table(file_entry_t **current) {
    file_entry_t entry;

    fread(&entry, sizeof(file_entry_t), 1, fp);
    if (entry.name[0] == ENTRY_EOF)
        return FALSE;

    file_entry_t *new_entry = (file_entry_t*) malloc(sizeof(file_entry_t));
    memcpy(new_entry, &entry, sizeof(file_entry_t));

    if (new_entry->children_count)
        new_entry->children = (file_entry_t**) malloc(sizeof(file_entry_t*) * new_entry->children_count);

    for (int i = 0; i < new_entry->children_count; i++) {
        if (!deserialize_file_table(&new_entry->children[i]))
            break;
    }

    *current = new_entry;

    return TRUE;
}

file_entry_t *find_directory(const char *filename) {
    if (!filename)
        return NULL;

    BOOL valid_path = TRUE;
    file_entry_t *current = root;
    char *duplicate = strdupa(filename), *token;

    while((token = strsep(&duplicate, "/")) && valid_path) {
        if (token[0] == '\0')
            continue;

        for (int i = 0; i < current->children_count; ++i) {
            if (!strcmp(token, current->children[i]->name) &&
                current->children[i]->attr == TYPE_DIRECTORY) {
                current = current->children[i];
                break;
            }
        }

        valid_path = valid_path && !strcmp(token, current->name);
    }

    return (valid_path) ? current : NULL;
}

file_entry_t *child_existed(file_entry_t *parent, const char *name) {
    for (int i = 0; i < parent->children_count; ++i) {
        if (!strcmp(name, parent->children[i]->name))
            return parent->children[i];
    }

    return NULL;
}

file_entry_t **realloc_entry_pointer_array(file_entry_t **orig, int new_size) {
    file_entry_t **result = (file_entry_t**) realloc(orig, new_size);
    if (result != orig)
        free(orig);

    return result;
}

void remove_child(file_entry_t *parent, const char *name) {
    if (!parent)
        return;

    int delete_index = 0;
    file_entry_t** new_children = NULL;

    if (parent->children_count > 1) {
        new_children = (file_entry_t**) malloc(sizeof(file_entry_t*) * (parent->children_count - 1));
        for (int i = 0, k = 0; i < parent->children_count; ++i) {
            if (strcmp(name, parent->children[i]->name))
                new_children[k++] = parent->children[i];
            else
                delete_index = i;
        }
    }

    free(parent->children[delete_index]);
    free(parent->children);
    parent->children = new_children;
    --parent->children_count;
}

void insert_child(file_entry_t *parent, file_entry_t new_entry) {
    if (!parent)
        return;

    if (!parent->children_count)
        parent->children = (file_entry_t**) malloc(sizeof(file_entry_t*));
    else
        parent->children = realloc_entry_pointer_array(parent->children, sizeof(file_entry_t*) * (parent->children_count + 1));

    parent->children[parent->children_count++] = (file_entry_t*) malloc(sizeof(file_entry_t));
    memcpy(parent->children[parent->children_count - 1], &new_entry, sizeof(file_entry_t));
}

void delete_data(int offset, int delete_size) {
    char move_buffer[BUFFER_SIZE];

    int move_size = filesystem_info.free_offset - (offset + delete_size);
    int read_offset = offset + delete_size;
    int write_offset = offset;
    while(move_size) {
        int current_move_size = (move_size < BUFFER_SIZE) ? move_size : BUFFER_SIZE;

        fseek(fp, read_offset, SEEK_SET);
        fread(move_buffer, 1, current_move_size, fp);

        fseek(fp, write_offset, SEEK_SET);
        fwrite(move_buffer, 1, current_move_size, fp);

        move_size -= current_move_size;
        read_offset += current_move_size;
        write_offset += current_move_size;
    }
}

int insert_data(int offset, const char *buffer, int insert_size) {
    char move_buffer[BUFFER_SIZE];

    int move_size = filesystem_info.free_offset - offset;
    int read_end_offset = filesystem_info.free_offset;
    while(move_size) {
        int current_move_size = (move_size < BUFFER_SIZE) ? move_size : BUFFER_SIZE;
        int read_offset = read_end_offset - current_move_size;
        int write_offset = read_offset + insert_size;

        fseek(fp, read_offset, SEEK_SET);
        fread(move_buffer, 1, current_move_size, fp);

        fseek(fp, write_offset, SEEK_SET);
        fwrite(move_buffer, 1, current_move_size, fp);

        move_size -= current_move_size;
        read_end_offset -= current_move_size;
    }

    fseek(fp, offset, SEEK_SET);
    return fwrite(buffer, 1, insert_size, fp);
}

void update_file_raw_offset(file_entry_t *current, file_entry_t *skip, int threshold, int offset) {
    if (!current)
        return;

    if (current != skip && current->raw_offset >= threshold)
        current->raw_offset += offset;

    for (int i = 0; i < current->children_count; ++i)
        update_file_raw_offset(current->children[i], skip, threshold, offset);
}

file_entry_t *valid_fd(int fd) {
    for (int i = 0; i < opened_file_count; ++i) {
        if (opened_file[i]->fd == fd)
            return opened_file[i];
    }

    return NULL;
}
