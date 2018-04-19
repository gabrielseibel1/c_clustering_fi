//
// Created by gabriel on 4/13/18.
//

#include "dendrogram.h"
#include <map>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <fstream>

int levels = 0;
std::map<int, cluster_t *> dendrogram;
int ids = 0;
std::map<cluster_t *, int> cluster_ids; //maps a cluster pointer to and id

void initialize_cluster_ids() {
    ids = 0;
    cluster_ids.insert(std::make_pair((cluster_t*) NULL, ids++));
}

int get_cluster_id(cluster_t* cluster_ptr) {
    std::map<cluster_t *, int>::iterator it = cluster_ids.find(cluster_ptr);

    if (it == cluster_ids.end()) {
        cluster_ids.insert(std::make_pair(cluster_ptr, ids++));
        return ids - 1;
    } else {
        return it->second;
    }
}

void split_points(int **points_1, int **points_2, int *points_1_size, int *points_2_size, int *points_membership,
                  int n_points, int *list_of_points) {
    /*std::cout << "POINTS_MEMBERSHIP: ";
    for (int point = 0; point < n_points; ++point) {
        std::cout << "[" << point << "] = " << points_membership[point] << " | ";
    }
    std::cout << "\n";*/

    //count how many points for each cluster
    (*points_1_size) = (*points_2_size) = 0;
    for (int i = 0; i < n_points; ++i) {
        switch (points_membership[i]) {
            default:
            case 0:
                (*points_1_size) = (*points_1_size) + 1;
                break;
            case 1:
                (*points_2_size) = (*points_2_size) + 1;
                break;
        }
    }

    //allocate two lists of points
    *points_1 = (int *) malloc(sizeof(int) * (*points_1_size));
    *points_2 = (int *) malloc(sizeof(int) * (*points_2_size));

    int points_1_index = 0;
    for (int point = 0; point < n_points; ++point) {
        if (points_membership[point] == 0) {
            (*points_1)[points_1_index] = list_of_points[point];
            ++points_1_index;
        }
    }

    int points_2_index = 0;
    for (int point = 0; point < n_points; ++point) {
        if (points_membership[point] == 1) {
            (*points_2)[points_2_index] = list_of_points[point];
            ++points_2_index;
        }
    }

    /*std::cout << "splits: { ";
    for (int point = 0; point < (*points_1_size); ++point) {
        std::cout << (*points_1)[point] << "__";
    }
    std::cout << " } and { ";
    for (int point = 0; point < (*points_2_size); ++point) {
        std::cout << (*points_2)[point] << "__";
    }
    std::cout << " }\n";*/
}

int are_all_clusters_in_level_unitary(int level) {
    std::map<int, cluster_t *>::iterator it = dendrogram.find(level);

    if (it == dendrogram.end()) { //could not find level
        std::cout << "Could not find level " << level << "\n";
        return false;
    } else { //found level
        cluster_t *cluster = it->second;
        bool all_are_unitary = true;
        while (all_are_unitary && cluster) {
            all_are_unitary = all_are_unitary && (cluster->size == 1);
            cluster = cluster->next_cluster;
        }
        return all_are_unitary;
    }
}

void inc_levels() { ++levels; }

int get_levels() { return levels; }

void initialize_dendrogram(cluster_t *father_cluster) {
    dendrogram.insert(std::make_pair(0, father_cluster));
    ++levels;
}

int split_cluster(int level, int *points_membership, cluster_t *original_cluster) {
    int *points_1 = NULL, *points_2 = NULL;
    int points_1_size, points_2_size;
    split_points(&points_1, &points_2, &points_1_size, &points_2_size, points_membership, original_cluster->size,
                 original_cluster->points);

    //if kmeans said all points are classified as same cluster, there is no need to split the original cluster in 2!!!
    if (points_1_size <= 0 || points_2_size <= 0) return false;

    cluster_t *cluster_1 = (cluster_t *) malloc(sizeof(cluster_t));
    cluster_1->points = points_1;
    cluster_1->size = points_1_size;
    cluster_1->father_cluster = original_cluster;
    cluster_1->left_child = NULL;
    cluster_1->right_child = NULL;

    cluster_t *cluster_2 = (cluster_t *) malloc(sizeof(cluster_t));
    cluster_2->points = points_2;
    cluster_2->size = points_2_size;
    cluster_2->father_cluster = original_cluster;
    cluster_2->left_child = NULL;
    cluster_2->right_child = NULL;

    //chain "brothers"
    cluster_1->next_cluster = cluster_2;
    cluster_2->next_cluster = NULL;

    //attach "brothers" to father
    original_cluster->left_child = cluster_1;
    original_cluster->right_child = cluster_2;

    std::map<int, cluster_t *>::iterator it = dendrogram.find(level);

    if (it == dendrogram.end()) { //could not find level - create new level and associate cluster to it
        ++levels;
        dendrogram.insert(std::make_pair(level, cluster_1));
    } else { //found level - get last cluster and append new clusters to it
        cluster_t *preceding_cluster = it->second;
        while (preceding_cluster->next_cluster)
            preceding_cluster = preceding_cluster->next_cluster;
        preceding_cluster->next_cluster = cluster_1;
    }

    /*std::cout << "Created two clusters: \n";
    print_cluster(cluster_1);
    print_cluster(cluster_2);*/

    return true;
}

cluster_t *get_cluster(int level, int cluster_index) {
    cluster_t *cluster = dendrogram.find(level)->second;
    for (int i = 0; i < cluster_index; ++i) {
        if (!cluster) break;
        cluster = cluster->next_cluster;
    }
    return cluster;
}

float **get_points_in_cluster(cluster_t *cluster, float **points, int n_features) {
    /*printf("ALL POINTS (cpp) (%d) << \n", 3);
    for (int k = 0; k < 3 *//*debug para o data_test.csv*//*; ++k) {
        for (int l = 0; l < n_features; ++l) {
            printf("%.2f - ", points[k][l]);
        }
        printf("\n");
    }
    printf(">>\n");*/

    //allocate space for returned matrix
    float **points_in_cluster = (float **) malloc(cluster->size * sizeof(float *));
    points_in_cluster[0] = (float *) calloc(static_cast<size_t>(cluster->size), n_features * sizeof(float));
    for (int i = 1; i < cluster->size; i++)
        points_in_cluster[i] = points_in_cluster[i - 1] + n_features;

    for (int point_index = 0; point_index < cluster->size; ++point_index) {
        for (int feature_index = 0; feature_index < n_features; ++feature_index) {
            points_in_cluster[point_index][feature_index] = points[cluster->points[point_index]][feature_index];
        }
    }

    /*printf("POINTS IN CLUSTER (cpp) (%d) << \n", cluster->size);
    for (int k = 0; k < cluster->size; ++k) {
        for (int l = 0; l < n_features; ++l) {
            printf("%.2f - ", points_in_cluster[k][l]);
        }
        printf("\n");
    }
    printf(">>\n");*/

    return points_in_cluster;
}

void print_cluster(cluster_t *cluster1) {
    std::cout << "CLUSTER #" << get_cluster_id(cluster1) << " {\n";
    std::cout << "\t\tpoints -> { ";
    for (int i = 0; i < cluster1->size; ++i) {
        std::cout << cluster1->points[i];
        if (i + 1 < cluster1->size) std::cout << ", ";
    }
    std::cout << " }\n";
    std::cout << "\t\tfather -> #" << get_cluster_id(cluster1->father_cluster) << "\n";
    std::cout << "\t\tbrother (next) -> #" << get_cluster_id(cluster1->next_cluster) << "\n";
    std::cout << "\t\tleft child -> #" << get_cluster_id(cluster1->left_child) << "\n";
    std::cout << "\t\tright child -> #" << get_cluster_id(cluster1->right_child) << "\n";
    std::cout << "\t}\n";
}

void print_dendrogram() {
    std::cout << "\n\nDENDROGRAM:\n";

    std::map<int, cluster_t *>::iterator it = dendrogram.begin();
    while (it != dendrogram.end()) {
        std::cout << "LEVEL " << it->first << " {\n";
        cluster_t *cluster = it->second;
        do {
            std::cout << "\t";
            print_cluster(cluster);
        } while ((cluster = cluster->next_cluster) != NULL);
        std::cout << "}\n";
        ++it;
    }
}

int dendrogram_to_file(char* filename) {
    std::ofstream file (filename);
    if (file.is_open()) {
        file << "DENDROGRAM:\n";

        std::map<int, cluster_t *>::iterator it = dendrogram.begin();
        while (it != dendrogram.end()) {
            file << "LEVEL " << it->first << " {\n";
            cluster_t *cluster = it->second;
            do {
                file << "\t";

                { //cluster to file
                    file << "CLUSTER #" << get_cluster_id(cluster) << " {\n";
                    file << "\t\tpoints -> { ";
                    for (int i = 0; i < cluster->size; ++i) {
                        file << cluster->points[i];
                        if (i + 1 < cluster->size) file << ", ";
                    }
                    file << " }\n";
                    file << "\t\tfather -> #" << get_cluster_id(cluster->father_cluster) << "\n";
                    file << "\t\tbrother (next) -> #" << get_cluster_id(cluster->next_cluster) << "\n";
                    file << "\t\tleft child -> #" << get_cluster_id(cluster->left_child) << "\n";
                    file << "\t\tright child -> #" << get_cluster_id(cluster->right_child) << "\n";
                    file << "\t}\n";
                }

            } while ((cluster = cluster->next_cluster) != NULL);
            file << "}\n";
            ++it;
        }

        file.close();
        return 0;

    } else {
        std::cout << "FAILED TO OPEN " << filename;

        return -1;
    }
}