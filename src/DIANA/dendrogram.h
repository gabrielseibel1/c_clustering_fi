//
// Created by gabriel on 4/13/18.
//

#ifndef C_CLUSTERING_DENDROGRAM_H
#define C_CLUSTERING_DENDROGRAM_H

#include <map>

typedef struct cluster {
    int size;
    int *points;
    struct cluster *father_cluster;
    struct cluster *next_cluster;
    struct cluster *left_child;
    struct cluster *right_child;

    //mocks for file io
    int id;
    int father_id;
    int brother_id;
    int left_child_id;
    int right_child_id;
} cluster_t;

void print_cluster(cluster_t *cluster1);
void print_dendrogram();
int dendrogram_to_binary_file(char* filename);
int dendrogram_to_text_file(char *filename);
int are_all_clusters_in_level_unitary(int level);
int get_levels();
void initialize_dendrogram(cluster_t* father_cluster);
void initialize_cluster_ids();
/**
 * Splits cluster in two smaller ones (if needed).
 * Returns whether there was a split (1) or not (0)
 */
int split_cluster(int level, int *points_membership, cluster_t *original_cluster);
cluster_t* get_cluster(int level, int cluster_index);
float **get_points_in_cluster(cluster_t* cluster, float **points, int n_features);
std::map<int, cluster_t*> *getOutputDendrogram();
std::map<int, cluster_t*> *dendrogramFromBinaryFile(char* filename);

#endif //C_CLUSTERING_DENDROGRAM_H
