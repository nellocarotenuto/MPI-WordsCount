#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include "workloads.h"
#include "wordsmap.h"
#include "logger.h"

char *log_execution_info(workloads_map *loads_map, words_map *words_map, double *execution_times) {
    struct stat stats;
    FILE *log_file;

    if (stat(LOGS_DIR, &stats)) {
        if (mkdir(LOGS_DIR, 0775)) {
            printf("Unable to create logs directory.\n");
            exit(1);
        }
    }

    time_t timestamp = time(NULL);
    struct tm tm = *localtime(&timestamp);
    char date[20];
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    char *file_name = calloc(64, sizeof(char));
    sprintf(file_name, "%s/%s -np %d", LOGS_DIR, date, loads_map->workers_count);

    log_file = fopen(file_name, "a+");

    if (!log_file) {
        printf("Unable to create log file\n");
        exit(1);
    }

    char dashed_line[LINE_WIDTH];

    for (int i = 0; i < LINE_WIDTH; i++) {
        dashed_line[i] = '-';
    }

    fprintf(log_file, "Execution info\n");
    fprintf(log_file, "%s\n", dashed_line);

    fprintf(log_file, "%-100s%20s\n", "Date of the test", date);
    fprintf(log_file, "%-100s%20d\n", "Number of workers", loads_map->workers_count);
    fprintf(log_file, "%-100s%20d\n", "Number of words", words_map->total_count);
    fprintf(log_file, "%-100s%20d\n", "Number of bytes", loads_map->total_size);
    fprintf(log_file, "%s\n\n", dashed_line);

    fprintf(log_file, "%s\n", "Execution times");
    fprintf(log_file, "%s\n", dashed_line);

    for (int i = 0; i < loads_map->workers_count; i++) {
        fprintf(log_file, "%6s%3d%110fs\n", "Worker", i, execution_times[i]);
    }

    fprintf(log_file, "%s\n\n", dashed_line);

    fprintf(log_file, "%s\n", "Workload distribution");
    fprintf(log_file, "%s\n", dashed_line);
    fprintf(log_file, "%6s %-95s %8s %8s\n", "Worker", "File", "Start", "End");
    fprintf(log_file, "%s\n", dashed_line);

    for (int i = 0; i < loads_map->workers_count; i++) {
        file_section_node *section = loads_map->lists[i];

        while (section) {
            fprintf(log_file, "%6d %-95s %8d %8d\n", i, section->file_name, section->start_index, section->end_index);
            section = section->next;
        }
    }

    fprintf(log_file, "%s\n\n", dashed_line);

    fprintf(log_file, "%s\n", "Results");
    fprintf(log_file, "%s\n", dashed_line);
    fprintf(log_file, "%-108s %11s\n", "Word", "Occurrences");
    fprintf(log_file, "%s\n", dashed_line);

    for (int i = 0; i < NUMBER_OF_LISTS; i++) {
        word_node *item = words_map->lists[i];

        while (item) {
            fprintf(log_file, "%-108s %11ld\n", item->word, item->count);
            item = item->next;
        }

    }

    fprintf(log_file, "%s\n\n", dashed_line);

    fclose(log_file);

    return file_name;
}
