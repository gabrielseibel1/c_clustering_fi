//
// Created by gabriel on 4/13/18.
//

#ifndef C_CLUSTERING_DENDROGRAM_H
#define C_CLUSTERING_DENDROGRAM_H

typedef struct cluster {
    int size;
    int *points;
    struct cluster *next_cluster;
    struct cluster *left_child;
    struct cluster *right_child;
} cluster_t;

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void print_cluster(cluster_t *cluster1);
EXTERNC void print_dendrogram();
EXTERNC int are_all_clusters_in_level_unitary(int level);
EXTERNC void inc_levels();
EXTERNC void initialize_dendrogram(cluster_t* father_cluster);
EXTERNC void insert_2_clusters_in_dendrogram(int level, int *points_membership, cluster_t* original_cluster);
EXTERNC cluster_t* get_cluster(int level, int cluster_index);
EXTERNC float **get_points_in_cluster(cluster_t* cluster, float **points, int n_features);

#undef EXTERNC

#endif //C_CLUSTERING_DENDROGRAM_H
