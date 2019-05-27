#define FILE_NAME_MAX_LENGTH 64

/*
 * Defines files metadata binding a file name to the actual file size.
 */
typedef struct file_info {
    char file_name[FILE_NAME_MAX_LENGTH];
    long size;
} file_info;

/*
 * Defines types to implement a map that allows to bind each file section to work on to a worker.
 */
typedef struct workloads_map {
    int workers_count;
    int *lists_length;
    struct file_section_node **lists;
} workloads_map;

typedef struct file_section_node {
    char file_name[FILE_NAME_MAX_LENGTH];
    int start_index;
    int end_index;
    struct file_section_node *next;
} file_section_node;

/*
 * Create a new workloads map for the specified number of workers and files and return its address to the caller.
 */
workloads_map *create_workloads_map(int workers_count, int files_count, char **files);
workloads_map *create_workloads_map_va(int workers_count, int files_count, ...);

/*
 * Print the content of the specified map.
 */
void print_workloads_map(workloads_map *map);

/*
 * Deallocate the map at the specified address,
 */
void free_workloads_map(workloads_map *map);
