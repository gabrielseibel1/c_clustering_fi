cmake_minimum_required(VERSION 3.9)
project(c_clustering)

set(CMAKE_CXX_STANDARD 11)
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

add_executable(c_clustering
        src/table.c
        include/table.h
        src/csv_parser.c
        include/csv_parser.h
        src/main.c)

add_executable(diana
        src/DIANA/diana.c
        src/DIANA/diana.h
        src/DIANA/diana_clustering.c
        src/kmeans/kmeans_clustering.c
        src/kmeans/kmeans_clustering.h
        src/DIANA/getopt.c
        src/DIANA/getopt.h
        src/DIANA/dendrogram.cpp
        src/DIANA/dendrogram.h
        src/table.c
        include/table.h
        src/csv_parser.c
        include/csv_parser.h)

add_executable(diana_gold_check
        src/DIANA/diana_gold_check.cpp)

add_executable(kmeans
        src/kmeans/kmeans.c
        src/kmeans/kmeans.h
        src/kmeans/cluster.c
        src/kmeans/cluster.h
        src/kmeans/kmeans_clustering.c
        src/kmeans/kmeans_clustering.h
        src/kmeans/getopt.c
        src/kmeans/getopt.h)

add_executable(datagen
        src/inpuGen/datagen.cpp)

add_executable(agnes
        src/AGNES/agglomerate.c)

target_link_libraries(agnes m)