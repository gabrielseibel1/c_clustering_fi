/**
 * Defines function headers for parsing CSV files
 */

#ifndef C_CLUSTERING_CSV_PARSER_H
#define C_CLUSTERING_CSV_PARSER_H

#include "../include/table.h"

t_table* csv_to_table(char* filename);
void csv_parser_test();

#endif //C_CLUSTERING_CSV_PARSER_H
