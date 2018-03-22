/**
 * Implements functions for parsing CSV files
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "../include/csv_parser.h"

t_table* csv_to_table(char* filename) {
    FILE* file_pointer = fopen(filename, "r");
    if (!file_pointer) {
        printf("Can't open %s in r mode.\n", filename);
        exit(EXIT_FAILURE);
    }

    //calculate file size
    fseek(file_pointer, 0, SEEK_END);
    size_t size = (size_t) ftell(file_pointer);
    fseek(file_pointer, 0, SEEK_SET);

    //copy content to memory
    char* file_content = calloc(size + 1, 1); //+1 for \0
    fread(file_content, size, 1, file_pointer);
    fclose(file_pointer);

    printf("File content: \n%s\n", file_content);
    free(file_content);
}

void csv_parser_test() {
    csv_to_table("../data/data_test.csv");
}
