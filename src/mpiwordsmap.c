#include <mpi.h>
#include <stdlib.h>
#include <string.h>

#include "wordsmap.h"
#include "mpiwordsmap.h"


void create_type_word(MPI_Datatype *word) {
    word_node node;
    MPI_Aint starting_address;
    MPI_Get_address(&node, &starting_address);

    int struct_length = 3;
    int block_lengths[struct_length];
    MPI_Datatype block_types[struct_length];
    MPI_Aint block_displacements[struct_length];

    block_lengths[0] = WORD_MAX_LENGTH;
    block_types[0] = MPI_CHAR;
    MPI_Get_address(&node.word, &block_displacements[0]);
    block_displacements[0] -= starting_address;

    block_lengths[1] = 1;
    block_types[1] = MPI_INT;
    MPI_Get_address(&node.count, &block_displacements[1]);
    block_displacements[1] -= starting_address;

    block_lengths[2] = 1;
    block_types[2] = MPI_AINT;
    MPI_Get_address(&node.next, &block_displacements[2]);
    block_displacements[2] -= starting_address;

    MPI_Type_create_struct(struct_length, block_lengths, block_displacements, block_types, word);
    MPI_Type_commit(word);
}


word_node *create_word_list_buffer(int list_length, word_node *list) {
    word_node *buffer = calloc(list_length, sizeof(word_node));
    word_node *node = list;

    for (int i = 0; i < list_length; i++) {
        strcpy(buffer[i].word, node->word);
        buffer[i].count = node->count;

        node = node->next;
    }

    return buffer;
}
