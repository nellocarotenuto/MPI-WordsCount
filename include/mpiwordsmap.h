/*
 * Create a datatype for the word count structure that MPI is aware of.
 */
void create_type_word(MPI_Datatype *word);

/*
 * Create a buffer of contiguous word counts to be used in MPI communications.
 */
word_node *create_word_list_buffer(int list_length, word_node *list);
