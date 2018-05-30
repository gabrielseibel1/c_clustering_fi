//
// Created by gabriel on 4/13/18.
//

#include "dendrogram.h"
#include <map>
#include <cstring>
#include <iostream>
#include <fstream>
#include <cmath>

std::map<int, cluster_t *> dendrogram;
std::map<cluster_t *, int> cluster_ids; //maps a cluster pointer to and id

void initialize_cluster_ids() {
    cluster_ids.insert(std::make_pair((cluster_t*) NULL, 0));
}

int get_cluster_id(cluster_t* cluster_ptr) {
    std::map<cluster_t *, int>::iterator it = cluster_ids.find(cluster_ptr);

    if (it == cluster_ids.end()) {
        --it;
        cluster_ids.insert(std::make_pair(cluster_ptr, it->second + 1));
        return it->second + 1;
    } else {
        return it->second;
    }
}

void split_points(int **points_1, int **points_2, int *points_1_size, int *points_2_size, int *points_membership,
                  int n_points, int *list_of_points) {
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

int get_levels() {
    dendrogram.size();
}

void initialize_dendrogram(cluster_t *father_cluster) {
    dendrogram.insert(std::make_pair(0, father_cluster));
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
        dendrogram.insert(std::make_pair(level, cluster_1));
    } else { //found level - get last cluster and append new clusters to it
        cluster_t *preceding_cluster = it->second;
        while (preceding_cluster->next_cluster)
            preceding_cluster = preceding_cluster->next_cluster;
        preceding_cluster->next_cluster = cluster_1;
    }
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

    auto it = dendrogram.begin();
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

int dendrogram_to_text_file(char *filename) {
    std::ofstream file (filename);
    if (file.is_open()) {
        file << "DENDROGRAM:\n";
        file << "LEVELS -> " << dendrogram.size() << "\n";
        auto it = dendrogram.begin();
        while (it != dendrogram.end()) {
            file << "LEVEL " << it->first << " {\n";
            cluster_t *cluster = it->second;
            do {
                file << "\t";

                { //cluster to file
                    file << "CLUSTER #" << get_cluster_id(cluster) << " {\n";
                    file << "\t\tfather -> #" << get_cluster_id(cluster->father_cluster) << "\n";
                    file << "\t\tbrother (next) -> #" << get_cluster_id(cluster->next_cluster) << "\n";
                    file << "\t\tleft child -> #" << get_cluster_id(cluster->left_child) << "\n";
                    file << "\t\tright child -> #" << get_cluster_id(cluster->right_child) << "\n";
                    file << "\t\tsize -> " << cluster->size << "\n";
                    file << "\t\tpoints -> { ";
                    for (int i = 0; i < cluster->size; ++i) {
                        file << cluster->points[i];
                        if (i + 1 < cluster->size) file << ", ";
                    }
                    file << " }\n";
                    file << "\t}\n";
                }

            } while ((cluster = cluster->next_cluster) != NULL);
            file << "}\n";
            ++it;
        }

        file.close();
        return 0;

    } else {
        std::cout << "FAILED TO OPEN " << filename << "\n";
        return -1;
    }
}

int dendrogram_to_binary_file(char* filename) {
    std::ofstream file (filename, std::ios::binary);
    if (file.is_open()) {
        auto dendroIt = dendrogram.begin();
        int levels = (int) dendrogram.size();
        file.write((char*) &levels, sizeof(int)); //levels
        while (dendroIt != dendrogram.end()) {
            int level = dendroIt->first;
            cluster_t *cluster = dendroIt->second;

            file.write((char*) &level, sizeof(int)); //level
            do {
                //cluster to file
                int cluster_id = get_cluster_id(cluster);
                int father_id = get_cluster_id(cluster->father_cluster);
                int brother_id = get_cluster_id(cluster->next_cluster);
                int left_child_id = get_cluster_id(cluster->left_child);
                int right_child_id = get_cluster_id(cluster->right_child);

                file.write((char*) &cluster->size, sizeof(int)); //cluster size
                file.write((char*) &cluster_id, sizeof(int)) ; //cluster id
                file.write((char*) &father_id, sizeof(int)); //father's id
                file.write((char*) &brother_id, sizeof(int)); //brother's id
                file.write((char*) &left_child_id, sizeof(int)); //left child's id
                file.write((char*) &right_child_id, sizeof(int)); //right child's id
                file.write((char*) cluster->points, cluster->size * sizeof(int)); //points

            } while ((cluster = cluster->next_cluster) != NULL);
            ++dendroIt;
        }

        file.close();
        return 0;

    } else {
        std::cout << "FAILED TO OPEN " << filename << "IN BINARY MODE\n";
        return -1;
    }
}

std::map<int, cluster_t*>* dendrogram_from_binary_file(char* filename) {
    auto *dendrogram = new std::map<int, cluster_t*>();
    std::ifstream file (filename, std::ios::binary);
    if (file.is_open()) {
        int levels;
        file.read((char*) &levels, sizeof(int)); //read levels
        for (int l = 0; l < levels; ++l) {
            int level;
            file.read((char*) &level, sizeof(int)); //level

            cluster_t *first_cluster = nullptr;
            cluster_t *cluster = nullptr;
            cluster_t *prev_cluster = nullptr;
            do {
                cluster = (cluster_t*) malloc(sizeof(cluster_t));
                cluster->next_cluster = nullptr;
                if (first_cluster == nullptr) first_cluster = cluster;

                file.read((char*) &cluster->size, sizeof(int)); //cluster size
                file.read((char*) &cluster->id, sizeof(int)) ; //cluster id
                file.read((char*) &cluster->father_id, sizeof(int)); //father's id
                file.read((char*) &cluster->brother_id, sizeof(int)); //brother's id
                file.read((char*) &cluster->left_child_id, sizeof(int)); //left child's id
                file.read((char*) &cluster->right_child_id, sizeof(int)); //right child's id

                cluster->points = (int*) malloc(cluster->size * sizeof(int));
                file.read((char*) cluster->points, cluster->size * sizeof(int)); //points

                if (prev_cluster != nullptr) { //requirement for reconstructing the dendrogram
                    prev_cluster->next_cluster = cluster;
                }
                prev_cluster = cluster;
            } while (cluster->brother_id != 0);

            dendrogram->insert(std::make_pair(level, first_cluster));
        }

        file.close();
        return dendrogram;

    } else {
        std::cout << "FAILED TO OPEN " << filename << " IN BINARY MODE [" << strerror(errno) << "]\n";
        return nullptr;
    }
}

int dendrogram_to_text_file(std::map<int, cluster_t*> *dendrogram, char *filename) {
    std::ofstream file (filename);
    if (file.is_open()) {
        file << "DENDROGRAM:\n";
        file << "LEVELS -> " << dendrogram->size() << "\n";
        std::map<int, cluster_t *>::iterator it = dendrogram->begin();
        while (it != dendrogram->end()) {
            file << "LEVEL " << it->first << " {\n";
            cluster_t *cluster = it->second;
            do {
                file << "\t";

                { //cluster to file
                    file << "CLUSTER #" << get_cluster_id(cluster) << " {\n";
                    file << "\t\tfather -> #" << get_cluster_id(cluster->father_cluster) << "\n";
                    file << "\t\tbrother (next) -> #" << get_cluster_id(cluster->next_cluster) << "\n";
                    file << "\t\tleft child -> #" << get_cluster_id(cluster->left_child) << "\n";
                    file << "\t\tright child -> #" << get_cluster_id(cluster->right_child) << "\n";
                    file << "\t\tsize -> " << cluster->size << "\n";
                    file << "\t\tpoints -> { ";
                    for (int i = 0; i < cluster->size; ++i) {
                        file << cluster->points[i];
                        if (i + 1 < cluster->size) file << ", ";
                    }
                    file << " }\n";
                    file << "\t}\n";
                }

            } while ((cluster = cluster->next_cluster) != NULL);
            file << "}\n";
            ++it;
        }

        file.close();
        return 0;

    } else {
        std::cout << "FAILED TO OPEN " << filename << "\n";
        return -1;
    }
}


