#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "workloads.h"

/*
 * Helper function that gets a map, a worker and a section and adds the section to the worker's workloads list in the
 * map.
 */
void add_section(workloads_map *map, int worker, file_section_node *section);


workloads_map *create_workloads_map_va(int workers_count, int files_count, ...) {
    char *files[files_count];

    va_list file_list;

    va_start(file_list, files_count);

    for (int i = 0; i < files_count; i++) {
        char *file_name = va_arg(file_list, char *);
        files[i] = file_name;
    }

    va_end(file_list);

    return create_workloads_map(workers_count, files_count, files);
}


workloads_map *create_workloads_map(int workers_count, int files_count, char **files) {
    workloads_map *map = malloc(sizeof(workloads_map *));
    map->workers_count = workers_count;
    map->lists = calloc(map->workers_count, sizeof(file_section_node *));
    map->lists_length = calloc(map->workers_count, sizeof(int));

    int total_size = 0;
    file_info infos[files_count];
    struct stat stats;

    for (int i = 0; i < files_count; i++) {
        char *file_name = files[i];

        if (strlen(file_name) > FILE_NAME_MAX_LENGTH) {
            printf("The file %s's name is too long.\n", file_name);
            exit(1);
        }

        if (access(file_name, R_OK)) {
            printf("The file %s could not be opened.\n", file_name);
            exit(1);
        }

        stat(file_name, &stats);

        strcpy(infos[i].file_name, file_name);
        infos[i].size = stats.st_size;

        total_size += stats.st_size;
    }

    map->total_size = total_size;

    int section_size = total_size / map->workers_count;
    int remainder = total_size % map->workers_count;

    int worker = 0;
    int worker_index = 0;

    for (int i = 0; i < files_count; i++) {
        int file_index = 0;

        while (file_index < infos[i].size) {
            file_section_node *section = calloc(1, sizeof(file_section_node));

            strcpy(section->file_name, infos[i].file_name);
            section->start_index = file_index;

            file_index += section_size - worker_index;

            if (file_index > infos[i].size) {
                section->end_index = infos[i].size;
            } else {
                if ((file_index + 1) <= infos[i].size && remainder > 0) {
                    file_index++;
                    remainder--;
                }

                section->end_index = file_index;
            }

            worker_index += section->end_index - section->start_index;

            add_section(map, worker, section);

            if (worker_index >= section_size) {
                worker++;
                worker_index = 0;
            }
        }

    }

    return map;
}


void print_workloads_map(workloads_map *map) {
    char dashed_line[91] = {0};

    for (int i = 0; i < 90; i++) {
        dashed_line[i] = '-';
    }
    printf("The total size of the files to analyze is %d bytes.\n", map->total_size);

    int section_size = map->total_size / map->workers_count;
    int remainder = map->total_size % map->workers_count;

    if (remainder == 0) {
        printf("The work load will be divided among workers in equal chunks of %d bytes as follows.\n",
                section_size);
    } else {
        printf("The work load will be divided among workers in chunks of ~%d bytes as follows.\n",
               section_size);
    }

    printf("%s\n", dashed_line);
    printf("%2s %-65s %10s %10s\n", "#W", "File", "Start", "End");
    printf("%s\n", dashed_line);

    for (int i = 0; i < map->workers_count; i++) {
        file_section_node *section = map->lists[i];

        while (section) {
            printf("%2d %-65s %10d %10d\n", i, section->file_name, section->start_index, section->end_index);
            section = section->next;
        }
    }

    printf("%s\n", dashed_line);
}


void free_workloads_map(workloads_map *map) {
    for (int i = 0; i < map->workers_count; i++) {
        file_section_node *section = map->lists[i];

        while (section) {
            file_section_node *temp = section;
            section = section->next;

            free(temp);
        }
    }

    free(map);
}


void add_section(workloads_map *map, int worker, file_section_node *section) {
    if (worker < 0 || worker >= map->workers_count) {
        printf("Unable to add section to the map. Requested worker (%d) must be between 0 and %d"
               ".\n", worker, map->workers_count);
        exit(1);
    }

    file_section_node *list = map->lists[worker];

    if (!list) {
        map->lists[worker] = section;
    } else {
        section->next = list;
        map->lists[worker] = section;
    }

    map->lists_length[worker]++;
}
