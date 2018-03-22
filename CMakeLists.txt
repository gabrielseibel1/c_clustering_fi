cmake_minimum_required(VERSION 3.9)
project(c_clustering C)

set(CMAKE_C_STANDARD 99)

add_executable(c_clustering src/table.c include/table.h src/csv_parser.c include/csv_parser.h src/main.c)