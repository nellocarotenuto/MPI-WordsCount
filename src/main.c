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

    int file_sections_list_length;
    file_section_node *file_sections_list;

    int words_list_length[size];
    word_node *words_list[size];

    MPI_Datatype type_file_section;
    create_type_file_section(&type_file_section);

    MPI_Datatype type_word;
    create_type_word(&type_word);

    MPI_Request file_list_length_request;
    MPI_Request words_list_length_request;

    workloads_map *loads_map;
    words_map *words_map;

    double execution_times[size];
    double starting_time = MPI_Wtime();

    char *log_file_name;

    if (rank == MASTER) {
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

        // Create a workloads map to determine who ha to do what and send the information to each worker
        loads_map = create_workloads_map(size, input->files_count, input->file_names);
        print_workloads_map(loads_map);

        for (int i = 0; i < input->files_count; i++) {
            free(input->file_names[i]);
        }

        free(input);

        file_sections_list_length = loads_map->lists_length[MASTER];
        file_sections_list = create_file_section_list_buffer(file_sections_list_length, loads_map->lists[MASTER]);

        for (int i = 1; i < size; i++) {
            int list_length = loads_map->lists_length[i];
            file_section_node *buffer = create_file_section_list_buffer(list_length, loads_map->lists[i]);

            MPI_Isend(&list_length, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &file_list_length_request);
            MPI_Send(buffer, list_length, type_file_section, i, 0, MPI_COMM_WORLD);

            free(buffer);
        }
    } else {
        // Receive the info about file sections to work on
        MPI_Recv(&file_sections_list_length, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        file_sections_list = calloc(file_sections_list_length, sizeof(file_section_node));
        MPI_Recv(file_sections_list, file_sections_list_length, type_file_section, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Create a words map and count the words in each file section assigned
    words_map = create_words_map();

    for (int i = 0; i < file_sections_list_length; i++) {
        count_words(file_sections_list[i].file_name, words_map, file_sections_list[i].start_index, file_sections_list[i].end_index);
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
                            update_words_map_with_count(words_map, words_list[j][k].word, words_list[j][k].count);
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
