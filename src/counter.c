#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wordsmap.h"

void count_words(char *file_name, words_map *map, int start_index, int end_index) {
    FILE *file;
    char c;
    int i;
    char buffer[256] = {0};

    if (start_index > end_index) {
        printf("The starting index must be smaller than the ending one.\n");
        exit(1);
    }

    if (!(file = fopen(file_name, "r"))) {
        printf("The file %s could not be opened.\n", file_name);
        exit(1);
    }

    if (start_index > 0) {
        if (fseek(file, start_index - 1, SEEK_SET)) {
            printf("Something went wrong while positioning the stream at the given offset.\n");
            exit(1);
        }

        c = getc(file);

        while (isalnum(c)) {
            c = getc(file);
        }
    }

    while (!feof(file) && ftell(file) < end_index) {
        i = 0;

        while (isalnum((c = getc(file)))) {
            buffer[i] = tolower(c);
            i++;
        }

        buffer[i] = '\0';

        if (strlen(buffer)) {
            update_words_map(map, buffer);
        }
    }

}
