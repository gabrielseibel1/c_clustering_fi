cmake_minimum_required(VERSION 3.9)
project(c_clustering)

set(CMAKE_C_STANDARD 99)

add_executable(c_clustering
        src/table.c
        include/table.h
        src/csv_parser.c
        include/csv_parser.h
        src/main.c)

add_executable(diana
        src/DIANA/cluster.c
        src/DIANA/diana.c
        src/DIANA/diana.h
        src/DIANA/diana_clustering.c
        src/DIANA/diana_gold_check.c
        src/DIANA/getopt.c
        src/DIANA/getopt.h)

add_executable(kmeans
        src/kmeans/cluster.c
        src/kmeans/kmeans.c
        src/kmeans/kmeans.h
        src/kmeans/kmeans_clustering.c
        src/kmeans/kmeans_gold_check.c
        src/kmeans/getopt.c
        src/kmeans/getopt.h)

add_executable(inpugen
        src/inpuGen/datagen.cpp)