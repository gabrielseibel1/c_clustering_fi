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

    //before we start tokenizing the content, we save a pointer to the start of the string, so we can free it later
    char* file_content_start = file_content;

    //create a table to store file content
    t_table* table = new_table(NULL);

    //get lines and fill a row out of each
    char* line;
    while ((line = strsep(&file_content, "\n")) && line[0] != '\0') {
        t_row* row = new_row(NULL);

        //each element between commas turns into a cell, appended to the row
        char* element;
        while ((element = strsep(&line, ","))) {
            t_cell* cell = new_cell(strtof(element, NULL));
            append_cell(row, cell);
        }
        //append filled row to table
        append_row(table, row);
    }

    print_table(table);
    clear_table(table);
    free(table);
    free(file_content_start);
}

void csv_parser_test() {
    csv_to_table("../data/data_test.csv");
}
