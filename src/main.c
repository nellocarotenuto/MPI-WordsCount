#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wordsmap.h>
#include <workloads.h>
#include <mpi.h>

#include "counter.h"
#include "logger.h"
#include "mpiwordsmap.h"
#include "mpiworkloads.h"

#define MASTER 0


int main(int argc, char *argv[]) {
    int size;
    int rank;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int file_sections_list_length;
    file_section_node *file_sections_list;

    int words_list_length[size];
    word_node *words_list[size];

    MPI_Datatype type_file_section;
    create_type_file_section(&type_file_section);

    MPI_Datatype type_word;
    create_type_word(&type_word);

    workloads_map *loads_map;
    words_map *words_map;

    double execution_times[size];
    double starting_time = MPI_Wtime();

    char *log_file_name;

    if (argc <= 1) {
        printf("Usage: mpirun -np %d ./MPI-WordsCount <filenames>\n", size);
        exit(1);
    }

    if (rank == MASTER) {
        int files_count = argc -1;
        char *files[files_count];

        for (int i = 1; i < argc; i++) {
            files[i - 1] = argv[i];
        }

        loads_map = create_workloads_map(size, files_count, files);

        for (int i = 0; i < size; i++) {
            int list_length = loads_map->lists_length[i];
            file_section_node *buffer = create_file_section_list_buffer(list_length, loads_map->lists[i]);

            MPI_Send(&list_length, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(buffer, list_length, type_file_section, i, 0, MPI_COMM_WORLD);

            free(buffer);
        }
    }

    MPI_Recv(&file_sections_list_length, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    file_sections_list = calloc(file_sections_list_length, sizeof(file_section_node));
    MPI_Recv(file_sections_list, file_sections_list_length, type_file_section, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    words_map = create_words_map();

    for (int i = 0; i < file_sections_list_length; i++) {
        count_words(file_sections_list[i].file_name, words_map, file_sections_list[i].start_index, file_sections_list[i].end_index);
    }

    for (int i = 0; i < NUMBER_OF_LISTS; i++) {
        int list_length = words_map->lists_length[i];
        MPI_Send(&list_length, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD);

        if (list_length > 0) {
            word_node *buffer = create_word_list_buffer(list_length, words_map->lists[i]);

            MPI_Send(buffer, list_length, type_word, MASTER, 0, MPI_COMM_WORLD);
            free(buffer);
        }
    }

    free_words_map(words_map);

    if (rank == MASTER) {
        words_map = create_words_map();

        for (int i = 0; i < NUMBER_OF_LISTS; i++) {
            for (int j = 0; j < size; j++) {
                MPI_Recv(&words_list_length[j], 1, MPI_INT, j, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                if (words_list_length[j] > 0) {
                    words_list[j] = calloc(words_list_length[j], sizeof(word_node));
                    MPI_Recv(words_list[j], words_list_length[j], type_word, j, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                    for (int k = 0; k < words_list_length[j]; k++) {
                        update_words_map_with_count(words_map, words_list[j][k].word, words_list[j][k].count);
                    }

                    free(words_list[j]);
                }
            }
        }
    }

    execution_times[rank] = MPI_Wtime() - starting_time;
    MPI_Gather(&execution_times[rank], 1, MPI_DOUBLE, &execution_times, 1, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);

    if (rank == MASTER) {
        print_words_map(words_map);
        log_file_name = log_execution_info(loads_map, words_map, execution_times);
        printf("Full report available at the following file: \"%s\".\n", log_file_name);

        free_words_map(words_map);
        free_workloads_map(loads_map);
    }

    MPI_Finalize();

    return 0;
}
