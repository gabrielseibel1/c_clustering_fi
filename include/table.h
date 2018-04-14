/**
 * Structs and headers for table.c
 * Defines a table, made of rows (which are made of cells). Each cell of the table contains data.
 * Defines structs, constructors, insertions, accesses (getter), printing and clearing for the data structures.
 */

#ifndef CSVPARSER_TABLE_H
#define CSVPARSER_TABLE_H

/**
 * Represents a table cell, containing data and a pointer to the next cell of the row
 */
typedef struct cell {
    float data;
    struct cell *next_cell;
} cell_t;

/**
 * Represents a row with a list of cells and a pointer to the next row of the table
 */
typedef struct row {
    cell_t *cells;
    struct row *next_row;
} row_t;

/**
 * Represents a table with a list of rows
 */
typedef struct table {
    row_t *rows;
} table_t;

/**
 * Constructor for t_cell.
 */
cell_t *new_cell(float data);

/**
 * Constructor for t_row. Accepts a NULL first_cell, if user wants to make an empty row.
 */
row_t *new_row(cell_t *first_cell);

/**
 * Constructor for t_table. Accepts a NULL first_row, if user wants to make an empty table.
 */
table_t *new_table(row_t *first_row);

/**
 * Appends a cell to the end of a row, making it the last cell.
 */
void append_cell(row_t *row, cell_t *cell_to_append);

/**
 * Appends a row to the end of a table, making it the last row.
 */
void append_row(table_t *table, row_t *row_to_append);

/**
 * Prints a t_cell.
 * @param cell_index is the index of the cell in it's row
 */
void print_cell(cell_t *cell, int row_index, int cell_index);

/**
 * Prints all the cells of the row, evoking print_cell(cell, cell_number) for each.
 * @param row_index is the index of the row in it's table
 */
void print_row(row_t *row, int row_index);

/**
 * Prints all of the rows of the table, evoking print_row(row, row_index) for each.
 */
void print_table(table_t *table);

/**
 * Clears allocated data from a cell
 */
void clear_cell(cell_t* cell);

/**
 * Clears (evokes clear_cell(cell)) and frees every cell of the row
 */
void clear_row(row_t* row);

/**
 * Clears (evokes clear_row(row)) and frees every row of the table
 */
void clear_table(table_t* table);

/**
 * Returns the table cell specified by the row and cell indexes. If it cannot find the cell, returns NULL.
 * @param row_index the number of the line containing the cell (starts from 0)
 * @param cell_index the number of the column containing the cell (starts from 0)
 * @return returns pointer to cell, if it can be found, or else returns NULL
 */
cell_t* get_cell(table_t* table, int row_index, int cell_index);

int count_rows(table_t* table);

int count_cells(row_t *row);

/**
 * Returns the contents of a table as float**
 */
float **points_from_table(table_t *table, int n_points, int n_attributes);

#endif //CSVPARSER_TABLE_H
