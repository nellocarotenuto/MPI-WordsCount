/*
 * Create a datatype for the file section structure that MPI is aware of.
 */
void create_type_file_section(MPI_Datatype *file_section);

/*
 * Create a buffer of contiguous file sections to be used in MPI communications.
 */
file_section_node *create_file_section_list_buffer(int list_length, file_section_node *list);
