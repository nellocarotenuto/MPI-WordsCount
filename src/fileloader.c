#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#include "fileloader.h"


input_files *load_files_from_directory(char *directory_name) {
    input_files *input = calloc(1, sizeof(input_files));
    int count = 0;

    int directory_name_length = strlen(directory_name);
    char file_base_path[NAME_MAX];
    strcpy(file_base_path, directory_name);

    if (file_base_path[directory_name_length - 1] != '/') {
        strcat(file_base_path, "/");
    }

    DIR *directory = opendir(directory_name);
    struct dirent *file;
    struct stat file_info;

    if (directory == NULL) {
        printf("Unable to access the directory %s.\n", directory_name);
        exit(1);
    }

    while ((file = readdir(directory))) {
        char file_name[NAME_MAX];
        strcpy(file_name, file_base_path);
        strcat(file_name, file->d_name);

        if (!stat(file_name, &file_info)) {
            if (S_ISREG(file_info.st_mode)) {
                if (strlen(file_name) >= FILE_NAME_MAX_LENGTH) {
                    printf("The following file's name is too long: %s.\n", file_name);
                    exit(1);
                }

                input->file_names[count] = calloc(strlen(file_name) + 1, sizeof(char));
                strcpy(input->file_names[count], file_name);
                count++;
            }
        } else {
            printf("Unable to read file info for %s.\n", file->d_name);
            exit(1);
        }
    }

    closedir(directory);

    input->files_count = count;
    return input;
}


input_files *load_files_from_master_file(char *index_file_name) {
    input_files *input = calloc(1, sizeof(input_files));
    int count = 0;

    FILE *index_file = fopen(index_file_name, "r");

    if (!index_file) {
        printf("Unable to read the index file %s.\n", index_file_name);
        exit(1);
    }

    char *line = NULL;
    size_t line_length;

    while (getline(&line, &line_length, index_file) != -1) {
        int line_index = 0;

        while (line[line_index] != '\n' && line[line_index] != '\0') {
            line_index++;
        }

        line[line_index] = '\0';

        if (line_index > 0) {
            if (line_index >= FILE_NAME_MAX_LENGTH) {
                printf("The name of the following file is too long: %s.\n", line);
                exit(1);
            }

            input->file_names[count] = calloc(line_index + 1, sizeof(char));
            strcpy(input->file_names[count], line);
            count++;
        }
    }

    fclose(index_file);

    if (line) {
        free(line);
    }

    input->files_count = count;
    return input;
}
