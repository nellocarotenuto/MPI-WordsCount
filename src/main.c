#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wordsmap.h>
#include <workloads.h>
#include <mpi.h>
#include <fileloader.h>

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

    file_section_node *file_section_node;

    int words_list_length[size];
    word_node *words_list[size];

    MPI_Datatype type_file_section;
    create_type_file_section(&type_file_section);

    MPI_Datatype type_word;
    create_type_word(&type_word);

    MPI_Request words_list_length_request;

    workloads_map *loads_map;
    words_map *words_map;

    double execution_times[size];
    double starting_time = MPI_Wtime();

    char *log_file_name;

    if (argc < 3) {
        printf("Possible usages:\n"
               "\tmpirun -np <processors> ./MPI-WordsCount -f <filenames>\n"
               "\tmpirun -np <processors> ./MPI-WordsCount -d <dirname>\n"
               "\tmpirun -np <processors> ./MPI-WordsCount -mf <masterfile>\n");
        exit(1);
    }

    // Fill the input files list
    input_files *input;

    if (!strcmp(argv[1], "-f")) {
        input = calloc(1, sizeof(input_files));
        input->files_count = argc - 2;

        for (int i = 2; i < argc; i++) {
            input->file_names[i - 2] = calloc(strlen(argv[i]) + 1, sizeof(char));
            strcpy(input->file_names[i - 2], argv[i]);
        }
    } else if (!strcmp(argv[1], "-d")) {
        input = load_files_from_directory(argv[2]);
    } else if (!strcmp(argv[1], "-mf")) {
        input = load_files_from_master_file(argv[2]);
    } else {
        printf("Unknown option \"%s\".\n", argv[1]);
        exit(1);
    }

    // Create a workloads map to determine who has to do what and send the information to each worker
    loads_map = create_workloads_map(size, input->files_count, input->file_names);

    if (rank == MASTER) {
        print_workloads_map(loads_map);
    }

    for (int i = 0; i < input->files_count; i++) {
        free(input->file_names[i]);
    }

    free(input);

    // Create a words map and count the words in each file section assigned
    words_map = create_words_map();

    file_section_node = loads_map->lists[rank];
    
    while (file_section_node) {
        count_words(file_section_node->file_name, words_map, file_section_node->start_index, file_section_node->end_index);
        file_section_node = file_section_node->next;
    }

    if (rank != MASTER) {
        free_workloads_map(loads_map);
    }

    if (size > 1) {
        if (rank == MASTER) {
            // Receive the lists of the wordsmap from each worker
            for (int i = 0; i < NUMBER_OF_LISTS; i++) {
                for (int j = 1; j < size; j++) {
                    MPI_Recv(&words_list_length[j], 1, MPI_INT, j, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                    if (words_list_length[j] > 0) {
                        words_list[j] = calloc(words_list_length[j], sizeof(word_node));
                        MPI_Recv(words_list[j], words_list_length[j], type_word, j, 0, MPI_COMM_WORLD,
                                 MPI_STATUS_IGNORE);

                        for (int k = 0; k < words_list_length[j]; k++) {
                            update_list(words_map, i, words_list[j][k].word, words_list[j][k].count);
                        }

                        free(words_list[j]);
                    }
                }
            }
        } else {
            // Send the lists of the hashmap
            for (int i = 0; i < NUMBER_OF_LISTS; i++) {
                int list_length = words_map->lists_length[i];
                MPI_Isend(&list_length, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD, &words_list_length_request);

                if (list_length > 0) {
                    word_node *buffer = create_word_list_buffer(list_length, words_map->lists[i]);

                    MPI_Send(buffer, list_length, type_word, MASTER, 0, MPI_COMM_WORLD);

                    free(buffer);
                }

                MPI_Wait(&words_list_length_request, MPI_STATUS_IGNORE);
            }

            free_words_map(words_map);
        }

    }

    // Compute the total execution times
    execution_times[rank] = MPI_Wtime() - starting_time;

    if (size > 1) {
        MPI_Gather(&execution_times[rank], 1, MPI_DOUBLE, &execution_times, 1, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
    }
    
    if (rank == MASTER) {
        // Output the results to console and write a log
        print_words_map(words_map);
        log_file_name = log_execution_info(loads_map, words_map, execution_times);

        double max_execution_time = execution_times[0];

        for (int i = 1; i < size; i++) {
            if (execution_times[i] > max_execution_time)
                max_execution_time = execution_times[i];
        }

        printf("Execution time: %fs.\nFull report available at \"%s\".\n", max_execution_time, log_file_name);

        free_words_map(words_map);
        free_workloads_map(loads_map);
    }

    MPI_Finalize();

    return 0;
}
