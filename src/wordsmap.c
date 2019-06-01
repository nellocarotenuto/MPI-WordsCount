#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/evp.h>
#include <openssl/sha.h>

#include "wordsmap.h"


#define HASH_ALGORITHM EVP_sha1()
#define DIGEST_LENGTH 20


/*
 * Helper function that computes the digest of a word and stores it in the location pointed by the third argument.
 */
void digest_word(const unsigned char *word, size_t word_length, unsigned char **digest, unsigned int *digest_length);

/*
 * Helper function that returns a pointer to the word_node in the list at the specified index of the map referring to the
 * passed word, if exists, NULL otherwise.
 */
word_node *lookup(const words_map *map, int list_index, const char *word);


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

    digest_length = DIGEST_LENGTH;
    digest = calloc(digest_length, sizeof(unsigned char));

    if (length >= WORD_MAX_LENGTH) {
        printf("The word %s is too long and can't be added to the map.\n", word);
        exit(1);
    }

    digest_word((unsigned char *) word, length, &digest, &digest_length);

    int list = digest[0] % NUMBER_OF_LISTS;
    free(digest);

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


void digest_word(const unsigned char *word, size_t word_length, unsigned char **digest, unsigned int *digest_length) {
    EVP_MD_CTX * digest_context = EVP_MD_CTX_create();

    if (!digest_context) {
        printf("Unable to create digest context for \"%s\".\n", word);
        exit(1);
    }

    if (!EVP_DigestInit_ex(digest_context, HASH_ALGORITHM, NULL)) {
        printf("Unable to set up digest context for \"%s\".\n", word);
        exit(1);
    }

    if(!EVP_DigestUpdate(digest_context, word, word_length)) {
        printf("Unable to update digest context for \"%s\".\n", word);
        exit(1);
    }

    *digest = (unsigned char *) OPENSSL_malloc(EVP_MD_size(HASH_ALGORITHM));

    if(*digest == NULL) {
        printf("Unable to compute digest for \"%s\".\n", word);
        exit(1);
    }

    if(!EVP_DigestFinal_ex(digest_context, *digest, digest_length)) {
        printf("Unable to finalize digest context for \"%s\".\n", word);
        exit(1);
    }

    EVP_MD_CTX_destroy(digest_context);
}
