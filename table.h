//
// Created by gabriel on 3/21/18.
//

#ifndef CSVPARSER_TABLE_H
#define CSVPARSER_TABLE_H

typedef struct cell {
    int data;
    struct cell *next_cell;
} t_cell;

typedef struct row {
    t_cell *cells;
    struct row *next_row;
} t_row;

typedef struct table {
    t_row *rows;
} t_table;

/**
 * Constructor for t_cell.
 */
t_cell *new_cell(int data);

/**
 * Constructor for t_row. Accepts a NULL first_cell, if user wants to make an empty row.
 */
t_row *new_row(t_cell *first_cell);

/**
 * Constructor for t_table. Accepts a NULL first_row, if user wants to make an empty table.
 */
t_table *new_table(t_row *first_row);

/**
 * Appends a cell to the end of a row, making it the last cell.
 */
void append_cell(t_row *row, t_cell *cell_to_append);

/**
 * Appends a row to the end of a table, making it the last row.
 */
void append_row(t_table *table, t_row *row_to_append);

/**
 * Prints a t_cell.
 * @param cell_index is the index of the cell in it's row
 */
void print_cell(t_cell *cell, int cell_index);

/**
 * Prints all the cells of the row, evoking print_cell(cell, cell_number) for each.
 * @param row_index is the index of the row in it's table
 */
void print_row(t_row *row, int row_index);

/**
 * Prints all of the rows of the table, evoking print_row(row, row_index) for each.
 */
void print_table(t_table *table);

/**
 * Clears allocated data from a cell
 */
void clear_cell(t_cell* cell);

/**
 * Clears (evokes clear_cell(cell)) and frees every cell of the row
 */
void clear_row(t_row* row);

/**
 * Clears (evokes clear_row(row)) and frees every row of the table
 */
void clear_table(t_table* table);

#endif //CSVPARSER_TABLE_H
