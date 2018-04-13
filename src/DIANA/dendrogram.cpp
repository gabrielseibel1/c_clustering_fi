//
// Created by gabriel on 4/13/18.
//

#include "dendrogram.h"
#include <map>
#include <cstdlib>
#include <cstring>
#include <iostream>

int levels = 0;
std::map<int, cluster_t *> dendrogram;

void split_points(int *points_1, int *points_2, int *points, int n_points) {
    //count how many points for each cluster
    int n_points_in_first = 0, n_points_in_second = 0;
    for (int i = 0; i < n_points; ++i) {
        switch (points[i]) {
            default:
            case 0:
                ++n_points_in_first;
                break;
            case 1:
                ++n_points_in_second;
                break;
        }
    }

    //allocate two lists of points
    points_1 = (int *) malloc(sizeof(int) * n_points_in_first);
    points_2 = (int *) malloc(sizeof(int) * n_points_in_second);

    for (int i = 0; i < n_points; ++i) {
        switch (points[i]) {
            default:
            case 0:
                points_1[i] = points[i];
                break;
            case 1:
                points_2[i] = points[i];
                break;
        }
    }
}

int are_all_clusters_in_level_unitary(int level) {
    std::map<int, cluster_t *>::iterator it = dendrogram.find(level);

    if (it == dendrogram.end()) { //could not find level
        std::cout << "Could not find level " << level << "\n";
        return false;
    } else { //found level
        cluster_t *cluster = it->second;
        bool all_are_unitary = true;
        while (all_are_unitary && cluster->next_cluster) {
            all_are_unitary = all_are_unitary && (cluster->size == 1);
            cluster = cluster->next_cluster;
        }
        return all_are_unitary;
    }
}

void reset_levels() { levels = 0; }

void inc_levels() { ++levels; }

void insert_2_clusters_in_dendrogram(int level, int *points, int n_points) {
    int *points_1 = NULL, *points_2 = NULL;

    split_points(points_1, points_2, points, n_points);

    cluster_t *cluster_1 = (cluster_t *) malloc(sizeof(cluster_t));
    cluster_1->points = points_1;
    cluster_1->size = sizeof(points_1) / sizeof(int);
    cluster_1->left_child = NULL;
    cluster_1->right_child = NULL;

    cluster_t *cluster_2 = (cluster_t *) malloc(sizeof(cluster_t));
    cluster_2->points = points_2;
    cluster_2->size = sizeof(points_2) / sizeof(int);
    cluster_2->left_child = NULL;
    cluster_2->right_child = NULL;

    cluster_1->next_cluster = cluster_2;
    cluster_2->next_cluster = NULL;

    std::map<int, cluster_t *>::iterator it = dendrogram.find(level);

    if (it == dendrogram.end()) { //could not find level - create new entry
        dendrogram.insert(std::make_pair(level, cluster_1));
    } else { //found level - get last cluster and append new clusters to it
        cluster_t *preceding_cluster = it->second;
        while (preceding_cluster->next_cluster)
            preceding_cluster = preceding_cluster->next_cluster;
        preceding_cluster->next_cluster = cluster_1;
    }
}

float **get_points_in_cluster(int level, int cluster_index, float **points, int n_features, int *n_points_in_cluster) {
    cluster_t *cluster = dendrogram.find(level)->second;
    for (int i = 0; i < cluster_index; ++i) {
        cluster = cluster->next_cluster;
    }
    *n_points_in_cluster = cluster->size;

    //allocate space for returned matrix
    float **points_in_cluster = (float **) malloc(cluster->size * sizeof(float *));
    points_in_cluster[0] = (float *) malloc(cluster->size * n_features * sizeof(float));
    for (int i = 1; i < cluster->size; i++)
        points_in_cluster[i] = points_in_cluster[i - 1] + n_features;

    //fill matrix of points x attributes with selected points
    for (int point = 0; point < cluster->size; ++point) {
        memcpy(points_in_cluster[point], points[point], n_features * sizeof(float));
    }
}
