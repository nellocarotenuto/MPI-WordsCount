/*
 * Defines files metadata binding a file name to the actual file size.
 */
typedef struct file_info {
    char *file_name;
    long size;
} file_info;

/*
 * Defines types to implement a map that allows to bind each file section to work on to a worker.
 */
typedef struct workloads_map {
    int workers_count;
    struct file_section **lists;
} workloads_map;

typedef struct file_section {
    char *file_name;
    int start_index;
    int end_index;
    struct file_section *next;
} file_section;

/*
 * Create a new workloads map for the specified number of workers and files and return its address to the caller.
 */
workloads_map *create_workloads_map(int workers_count, int files_count, ...);

/*
 * Print the content of the specified map.
 */
void print_workloads_map(workloads_map *map);

/*
 * Deallocate the map at the specified address,
 */
void free_workloads_map(workloads_map *map);
