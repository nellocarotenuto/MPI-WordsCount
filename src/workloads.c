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


workloads_map *create_workloads_map(int workers_count, int files_count, ...) {
    workloads_map *map = malloc(sizeof(workloads_map *));
    map->workers_count = workers_count;
    map->lists = calloc(map->workers_count, sizeof(file_section_node *));
    map->lists_length = calloc(map->workers_count, sizeof(int));

    int total_size = 0;
    file_info infos[files_count];
    struct stat stats;

    va_list file_list;

    va_start(file_list, files_count);

    for (int i = 0; i < files_count; i++) {
        char *file_name = va_arg(file_list, char *);

        if (access(file_name, R_OK)) {
            printf("The file %s could not be opened.\n", file_name);
            exit(1);
        }

        stat(file_name, &stats);

        strcpy(infos[i].file_name, file_name);
        infos[i].size = stats.st_size;

        total_size += stats.st_size;
    }

    va_end(file_list);

    int section_size = total_size / map->workers_count;
    int remainder = total_size % map->workers_count;

    int worker = 0;
    int worker_index = 0;

    for (int i = 0; i < files_count; i++) {
        int file_index = 0;

        while (file_index < infos[i].size) {
            file_section_node *section = malloc(sizeof(file_section_node));

            strcpy(section->file_name, infos[i].file_name);
            section->start_index = file_index;

            file_index += section_size - worker_index;

            if (remainder > 0) {
                file_index++;
                remainder--;
            }

            if (file_index > infos[i].size) {
                section->end_index = infos[i].size;
            } else {
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
    printf("%6s %-101s %5s %5s\n", "Worker", "File", "Start", "End");

    for (int i = 0; i < 120; i++) {
        printf("-");
    }

    printf("\n\n");

    for (int i = 0; i < map->workers_count; i++) {
        file_section_node *section = map->lists[i];

        while (section) {
            printf("%6d %-101s %5d %5d\n", i, section->file_name, section->start_index, section->end_index);
            section = section->next;
        }
    }
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
        printf("Unable to add section to the map because the worker to assign it must be non-negative and less than %d"
               ".\n", map->workers_count);
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
