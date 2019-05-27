#include <mpi.h>
#include <stdlib.h>
#include <string.h>

#include "workloads.h"
#include "mpiworkloads.h"


void create_type_file_section(MPI_Datatype *file_section) {
    file_section_node node;
    MPI_Aint starting_address;
    MPI_Get_address(&node, &starting_address);

    int struct_length = 4;
    int block_lengths[struct_length];
    MPI_Datatype block_types[struct_length];
    MPI_Aint block_displacements[struct_length];

    block_lengths[0] = FILE_NAME_MAX_LENGTH;
    block_types[0] = MPI_CHAR;
    MPI_Get_address(&node.file_name, &block_displacements[0]);
    block_displacements[0] -= starting_address;

    block_lengths[1] = 1;
    block_types[1] = MPI_INT;
    MPI_Get_address(&node.start_index, &block_displacements[1]);
    block_displacements[1] -= starting_address;

    block_lengths[2] = 1;
    block_types[2] = MPI_INT;
    MPI_Get_address(&node.end_index, &block_displacements[2]);
    block_displacements[2] -= starting_address;

    block_lengths[3] = 1;
    block_types[3] = MPI_AINT;
    MPI_Get_address(&node.next, &block_displacements[3]);
    block_displacements[3] -= starting_address;

    MPI_Type_create_struct(struct_length, block_lengths, block_displacements, block_types, file_section);
    MPI_Type_commit(file_section);
}


file_section_node *create_file_section_list_buffer(int list_length, file_section_node *list) {
    file_section_node *buffer = calloc(list_length, sizeof(file_section_node));
    file_section_node *node = list;

    for (int i = 0; i < list_length; i++) {
        strcpy(buffer[i].file_name, node->file_name);
        buffer[i].start_index = node->start_index;
        buffer[i].end_index = node->end_index;

        node = node->next;
    }

    return buffer;
}
