#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wordsmap.h"


/*
 * Helper function that returns a pointer to the word_node in the list at the specified index of the map referring to the
 * passed word, if exists, NULL otherwise.
 */
word_node *lookup(const words_map *map, int list_index, const char *word);

/*
 * Helper function that computes the digest of a word applying Pearson Hashing to the parameter and returning a byte.
 */
unsigned char digest_word(const unsigned char *word, int length);


words_map *create_words_map() {
    words_map *map = malloc(sizeof(words_map *));
    map->lists = calloc(NUMBER_OF_LISTS, sizeof(word_node *));
    map->lists_length = calloc(NUMBER_OF_LISTS, sizeof(int));
    map->total_count = 0;

    return map;
}


void update_words_map(words_map *map, const char *word) {
    update_words_map_with_count(map, word, 1);
}


void update_words_map_with_count(words_map *map, const char *word, int count) {
    unsigned int digest_length;
    unsigned char *digest;

    int length = strlen(word);

    if (length == 0) {
        return;
    }

    if (length >= WORD_MAX_LENGTH) {
        printf("The word %s is too long and can't be added to the map.\n", word);
        exit(1);
    }

    int list = digest_word(word, length) % NUMBER_OF_LISTS;

    update_list(map, list, word, count);

    map->total_count += count;
}


void update_list(words_map *map, int list, const char *word, int count) {
    word_node *item = lookup(map, list, word);

    if (!item) {
        item = calloc(1, sizeof(word_node));

        item->next = map->lists[list];
        map->lists[list] = item;

        strcpy(item->word, word);

        item->count = count;
        map->lists_length[list]++;
    } else {
        item->count += count;
    }
}


void print_words_map(words_map *map) {
    char dashed_line[91] = {0};

    for (int i = 0; i < 90; i++) {
        dashed_line[i] = '-';
    }

    printf("%-78s %11s\n", "Word", "Occurrences");
    printf("%s\n", dashed_line);

    for (int i = 0; i < NUMBER_OF_LISTS; i++) {
        word_node *item = map->lists[i];

        while (item) {
            printf("%-78s %11d\n", item->word, item->count);
            item = item->next;
        }

    }

    printf("%s\n", dashed_line);
}


words_map *merge_words_maps(int maps_count, ...) {
    words_map *map = create_words_map();

    va_list maps_list;

    va_start(maps_list, maps_count);

    for (int i = 0; i < maps_count; i++) {
        words_map *arg_map = va_arg(maps_list, words_map *);

        for (int j = 0; j < NUMBER_OF_LISTS; j++) {
            word_node *item = arg_map->lists[j];

            while (item) {
                update_words_map_with_count(map, item->word, item->count);
                item = item->next;
            }
        }
    }

    va_end(maps_list);

    return map;
}


words_map *merge_words_maps_array(int maps_count, words_map **maps) {
    words_map *map = create_words_map();

    for (int i = 0; i < maps_count; i++) {
        words_map *arg_map = maps[i];

        for (int j = 0; j < NUMBER_OF_LISTS; j++) {
            word_node *item = arg_map->lists[j];

            while (item) {
                update_words_map_with_count(map, item->word, item->count);
                item = item->next;
            }
        }
    }

    return map;
}


word_node *lookup(const words_map *map, int list_index, const char *word) {
    word_node *item = map->lists[list_index];

    if (!item) {
        return NULL;
    }

    while (strcmp(item->word, word)) {
        if (!item->next) {
            return NULL;
        }

        item = item->next;
    }

    return item;
}


void free_words_map(words_map *map) {
    for (int i = 0; i < NUMBER_OF_LISTS; i++) {
        word_node *item = map->lists[i];

        while (item) {
            word_node *temp = item;
            item = item->next;

            free(temp);
        }
    }

    free(map);
}

unsigned char digest_word(const unsigned char *word, int length) {
    unsigned char h;

    static const unsigned char T[256] = {
         98,  6, 85,150, 36, 23,112,164,135,207,169,  5, 26, 64,165,219,
         61, 20, 68, 89,130, 63, 52,102, 24,229,132,245, 80,216,195,115,
         90,168,156,203,177,120,  2,190,188,  7,100,185,174,243,162, 10,
        237, 18,253,225,  8,208,172,244,255,126,101, 79,145,235,228,121,
        123,251, 67,250,161,  0,107, 97,241,111,181, 82,249, 33, 69, 55,
         59,153, 29,  9,213,167, 84, 93, 30, 46, 94, 75,151,114, 73,222,
        197, 96,210, 45, 16,227,248,202, 51,152,252,125, 81,206,215,186,
         39,158,178,187,131,136,  1, 49, 50, 17,141, 91, 47,129, 60, 99,
        154, 35, 86,171,105, 34, 38,200,147, 58, 77,118,173,246, 76,254,
        133,232,196,144,198,124, 53,  4,108, 74,223,234,134,230,157,139,
        189,205,199,128,176, 19,211,236,127,192,231, 70,233, 88,146, 44,
        183,201, 22, 83, 13,214,116,109,159, 32, 95,226,140,220, 57, 12,
        221, 31,209,182,143, 92,149,184,148, 62,113, 65, 37, 27,106,166,
          3, 14,204, 72, 21, 41, 56, 66, 28,193, 40,217, 25, 54,179,117,
        238, 87,240,155,180,170,242,212,191,163, 78,218,137,194,175,110,
         43,119,224, 71,122,142, 42,160,104, 48,247,103, 15, 11,138,239
    };

    h = T[word[0]];

    for (size_t i = 1; i < length; ++i) {
        h = T[h ^ word[i]];
    }

    return h;
}
