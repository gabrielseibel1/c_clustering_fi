/**
 * Implementations for table.h
 * Implements functions for tables, made of rows (which are made of cells). Each cell of the table contains data.
 * Implements constructors, insertions, accesses (getter), printing and clearing for the data structures.
 */

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include "../include/table.h"

cell_t *new_cell(float data) {
    cell_t *cell = (cell_t *) malloc(sizeof(cell_t));
    cell->data = data;
    cell->next_cell = NULL;
    return cell;
}

row_t *new_row(cell_t *first_cell) {
    row_t *row = (row_t *) malloc(sizeof(row_t));
    row->cells = first_cell;
    row->next_row = NULL;
    return row;
}

table_t *new_table(row_t *first_row) {
    table_t *table = (table_t *) malloc(sizeof(table_t));
    table->rows = first_row;
    return table;
}

void append_cell(row_t *row, cell_t *cell_to_append) {
    if (!row) {
        fprintf(stderr, "Row is NULL, can't append cell.\n");
        exit(EXIT_FAILURE);
    }

    //if there are no cells, append as first one
    if (!row->cells) {
        row->cells = cell_to_append;
        return;
    }

    //look for last cell of the row
    cell_t *current_cell = row->cells;
    while (current_cell->next_cell) {
        current_cell = current_cell->next_cell;
    }
    //here, current_cell is the last cell, so we append to it
    current_cell->next_cell = cell_to_append;
}

void append_row(table_t *table, row_t *row_to_append) {
    if (!table) {
        fprintf(stderr, "Table is NULL, can't append row.\n");
        exit(EXIT_FAILURE);
    }

    //if there are no rows, append as first one
    if (table->rows == NULL) {
        table->rows = row_to_append;
        return;
    }

    //look for last row of the table
    row_t *current_row = table->rows;
    while (current_row->next_row) {
        current_row = current_row->next_row;
    }
    //here, current_row is the last row, so we append to it
    current_row->next_row = row_to_append;
}

void print_cell(cell_t *cell, int row_index, int cell_index) {
    if (!cell) {
        fprintf(stderr, "Cell is NULL, can't print it.\n");
        exit(EXIT_FAILURE);
    }
    printf("Cell [%d][%d] { data: %f } ", row_index, cell_index, cell->data);
}

void print_row(row_t *row, int row_index) {
    if (!row) {
        fprintf(stderr, "Row is NULL, can't print it.\n");
        exit(EXIT_FAILURE);
    }

    printf("Row %d { ", row_index);
    cell_t *cell = row->cells;
    int cell_index = 0;
    while (cell) {
        print_cell(cell, row_index, cell_index);
        cell = cell->next_cell;
        ++cell_index;
    }
    printf("}\n");
}

void print_table(table_t *table) {
    if (!table) {
        fprintf(stderr, "Table is NULL, can't print it.\n");
        exit(EXIT_FAILURE);
    }

    row_t *row = table->rows;
    int index = 0;
    while (row) {
        print_row(row, index);
        row = row->next_row;
        ++index;
    }
}

void clear_cell(cell_t *cell) {
    //nothing to be cleared
}

void clear_row(row_t *row) {
    if (!row) {
        fprintf(stderr, "Row is NULL, can't clear it.\n");
        exit(EXIT_FAILURE);
    }

    cell_t *cell = row->cells;
    while (cell) {
        clear_cell(cell);
        cell_t *next_cell = cell->next_cell;
        free(cell);
        cell = next_cell;
    }
}

void clear_table(table_t *table) {
    if (!table) {
        fprintf(stderr, "Table is NULL, can't clear it.\n");
        exit(EXIT_FAILURE);
    }

    row_t *row = table->rows;
    while (row) {
        clear_row(row);
        row_t *next_row = row->next_row;
        free(row);
        row = next_row;
    }
}

cell_t *get_cell(table_t *table, int row_index, int cell_index) {
    if (!table) {
        fprintf(stderr, "Table is NULL, can't get cell [%d][%d].\n", row_index, cell_index);
        exit(EXIT_FAILURE);
    }

    //find row
    row_t *row = table->rows;
    for (int i = 0; i < row_index; ++i) {
        if (!row) return NULL;
        row = row->next_row;
    }
    if (!row) return NULL;

    //find cell
    cell_t *cell = row->cells;
    for (int j = 0; j < cell_index; ++j) {
        if (!cell) return NULL;
        cell = cell->next_cell;
    }
    if (!cell) return NULL;

    return cell;
}

int count_rows(table_t *table) {
    if (!table) {
        fprintf(stderr, "Table is NULL, can't count rows.\n");
        exit(EXIT_FAILURE);
    }

    row_t *row = table->rows;
    int count = 0;
    while (row != NULL) {
        ++count;
        row = row->next_row;
    }

    return count;
}

int count_cells(row_t *row) {
    if (!row) {
        fprintf(stderr, "Row is NULL, can't count cells.\n");
        exit(EXIT_FAILURE);
    }

    cell_t *cell = row->cells;
    int count = 0;
    while (cell != NULL) {
        ++count;
        cell = cell->next_cell;
    }

    return count;
}

float **points_from_table(table_t *table, int n_points, int n_attributes) {
    if (!table) {
        fprintf(stderr, "Table is NULL, can't count rows.\n");
        exit(EXIT_FAILURE);
    }

    //allocate space for points
    float** points = (float **) malloc(n_points * sizeof(float *));
    points[0] = (float *) malloc(n_points * n_attributes * sizeof(float));
    for (int i = 1; i < n_points; i++) {
        points[i] = points[i - 1] + n_attributes;
    }

    for (int i = 0; i < n_points; ++i) {
        for (int j = 0; j < n_attributes; ++j) {
            points[i][j] = get_cell(table, i, j)->data;
        }
    }
}

int table_test() {
    table_t *table = new_table(NULL);
    for (int i = 0; i < 5; ++i) {
        row_t *row = new_row(NULL);
        for (int j = 0; j < 3; ++j) {
            append_cell(row, new_cell(j));
        }
        append_row(table, row);
    }

    printf("Testing print_table(table):\n");
    print_table(table);
    printf("\n");

    printf("Testing get_cell(cell, i, j):\n");
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 4; ++j) {
            cell_t *cell = get_cell(table, i, j);
            if (cell) print_cell(cell, i, j);
        }
        printf("\n");
    }

    clear_table(table);
    free(table);

    return 0;
}