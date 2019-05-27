#define WORD_MAX_LENGTH 64

/*
 * Defines types to implement a simple hashmap with a variable number of buckets each of which contains a list of words
 * whose digest's first (unsigned) character modulo the number of buckets equals to bucket's index.
 */
typedef struct words_map {
    struct word_node **lists;
    int *lists_length;
} words_map;

typedef struct word_node {
    char word[WORD_MAX_LENGTH];
    int count;
    struct word_node *next;
} word_node;

/*
 *  Create a new words map and return its address to the caller.
 */
words_map *create_words_map(void);

/*
 * Update the map at the specified address with the passed word. This function adds the word to the map if it does not
 * already exist in the latter or updates its count otherwise.
 */
void update_words_map(words_map *map, const char *word);

/*
 * Update the map at the specified address with the passed word. This function adds the word with the passed number of
 * occurrences to the map if it does not already exist in the latter or updates its count otherwise.
 */
void update_words_map_with_count(words_map *map, const char *word, int count);

/*
 * Print the content of the specified map.
 */
void print_words_map(words_map *map);

/*
 * Create a new map containing the words of the maps passed as parameters and return its address.
 * This function doesn't deallocate the maps passed as arguments.
 */
words_map *merge_words_maps(int maps_count, ...);


/*
 * Create a new map containing the words of the maps passed in the array parameter and return its address.
 * This function doesn't deallocate the maps passed as arguments.
 */
words_map *merge_words_maps_array(int maps_count, words_map **maps);

/*
 * Deallocate the map at the specified address,
 */
void free_words_map(words_map *map);
