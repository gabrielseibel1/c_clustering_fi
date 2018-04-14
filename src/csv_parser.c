/**
 * Implements functions for parsing CSV files
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "../include/csv_parser.h"

table_t* csv_to_table(char* filename) {
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
    table_t* table = new_table(NULL);

    //get lines and fill a row out of each
    char* line;
    while ((line = strsep(&file_content, "\n")) && line[0] != '\0') {
        row_t* row = new_row(NULL);

        //each element between commas turns into a cell, appended to the row
        char* element;
        while ((element = strsep(&line, " ,"))) {
            if (strcmp(element, "\0") != 0) {
                cell_t* cell = new_cell(strtof(element,NULL));
                append_cell(row, cell);
            }
        }
        //append filled row to table
        append_row(table, row);
    }
    //free bytes read from file
    free(file_content_start);

    return table;
}

void csv_parser_test() {
    printf("test_data.csv:\n");
    table_t* table_test = csv_to_table("../data/data_test.csv");
    print_table(table_test);
    clear_table(table_test);
    free(table_test);

    //read haberman's survival data set
    printf("\n\nhaberman.data:\n");
    table_t* table_haberman = csv_to_table("../data/haberman.data");
    print_table(table_haberman);
    clear_table(table_haberman);
    free(table_haberman);
}
