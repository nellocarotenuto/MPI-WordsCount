#define FILE_MAX_COUNT 64
#define FILE_NAME_MAX_LENGTH 64

typedef struct input_files {
    int files_count;
    char *file_names[FILE_MAX_COUNT];
} input_files;

/*
 * Stores the list of file names contained in the specified folder into the last parameter.
 */
input_files *load_files_from_directory(char *directory_name);

/*
 * Stores the list of file names contained in the specified index file.
 */
input_files *load_files_from_master_file(char *index_file_name);
