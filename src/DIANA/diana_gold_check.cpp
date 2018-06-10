//
// Created by gabriel on 5/30/18.
//

#include <map>
#include <fstream>
#include <iostream>
#include <cstring>
#include <omp.h>
#include "dendrogram.h"
#include "getopt.h"

std::map<int, cluster_t*>* dendrogramFromBinaryFile(char *filename) {
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

int dendrogram_to_text_file_mock(std::map<int, cluster_t*> *dendrogram, char *filename) {
    std::ofstream file (filename);
    if (file.is_open()) {
        file << "DENDROGRAM:\n";
        int levels = (int) dendrogram->size();
        auto it = dendrogram->begin();
        file << "LEVELS -> " << levels << "\n";
        while (it != dendrogram->end()) {
            file << "LEVEL " << it->first << " {\n";

            cluster_t *cluster = it->second;
            do {
                //cluster to file
                file << "\t";
                file << "CLUSTER #" << cluster->id << " {\n";
                file << "\t\tfather -> #" << cluster->father_id << "\n";
                file << "\t\tbrother (next) -> #" << cluster->brother_id << "\n";
                file << "\t\tleft child -> #" << cluster->left_child_id << "\n";
                file << "\t\tright child -> #" << cluster->right_child_id << "\n";
                file << "\t\tsize -> " << cluster->size << "\n";
                file << "\t\tpoints -> { ";
                for (int i = 0; i < cluster->size; ++i) {
                    file << cluster->points[i];
                    if (i + 1 < cluster->size) file << ", ";
                }
                file << " }\n";
                file << "\t}\n";

            } while ((cluster = cluster->next_cluster) != nullptr);
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

void flagIfDifferent(int a, int b, int level, int cluster_count, int *errors, char *description) {
    if (a != b) {
        printf("SDC: [L:%d, N:%d] %s -> OUT: %d , GOLD: %d\n",
               level, cluster_count, description, a, a);
        ++(*errors);
    }
}

int compare_out_and_gold(char *out_filename, char *gold_filename) {
    int errors = 0;

    std::map<int, cluster_t*> *dendrogram_out = dendrogramFromBinaryFile(out_filename);
    std::map<int, cluster_t*> *dendrogram_gold = dendrogramFromBinaryFile(gold_filename);

    //TODO remove
    dendrogram_to_text_file_mock(dendrogram_out, const_cast<char *>("diana_feedback_out.txt"));
    dendrogram_to_text_file_mock(dendrogram_gold, const_cast<char *>("diana_feedback_gold.txt"));

    if (dendrogram_out == nullptr || dendrogram_gold == nullptr) return -1;

    //retrieve level count
    int levels_out = (int) dendrogram_out->size();
    int levels_gold = (int) dendrogram_gold->size();

    //check equal level count
    if (levels_out != levels_gold) {
        printf("SDC: Levels -> OUT: %d , GOLD: %d\n", levels_out, levels_gold);
        ++errors;
    }

    auto iter_out = dendrogram_out->begin();
    auto iter_gold = dendrogram_gold->begin();

    //here we loop for "the smallest level" times, so all dendrogram operations are safe (all levels exist).
    //if the sizes are the same, compares all levels
    int smallest_level = (levels_gold <= levels_out) ? levels_gold : levels_out;
    for (int level = 0; level < smallest_level; ++level) {
        cluster_t *cluster_out = iter_out->second;
        cluster_t *cluster_gold = iter_gold->second;

        int cluster_count = 0;
        bool clusters_are_not_nullptr = (cluster_out != nullptr) && (cluster_gold != nullptr);
        while (clusters_are_not_nullptr) {

            //check equal cluster size
            flagIfDifferent(cluster_out->size, cluster_gold->size,
                            level, cluster_count, &errors, (char *)("Cluster size"));
            //check equal id
            flagIfDifferent(cluster_out->id, cluster_gold->id,
                            level, cluster_count, &errors, (char *) "Cluster id");
            //check equal father id
            flagIfDifferent(cluster_out->father_id, cluster_gold->father_id,
                            level, cluster_count, &errors, (char *) "Father id");
            //check equal brother id
            flagIfDifferent(cluster_out->brother_id, cluster_gold->brother_id,
                            level, cluster_count, &errors, (char *) "Brother id");
            //check equal left child id
            flagIfDifferent(cluster_out->left_child_id, cluster_gold->left_child_id,
                            level, cluster_count, &errors, (char *) "Left Child id");
            //check equal right child id
            flagIfDifferent(cluster_out->right_child_id, cluster_gold->right_child_id,
                            level, cluster_count, &errors, (char *) "Right Child id");

            //check equal points. Here we loop for "the smallest cluster size" times,
            // so all points[i] operations are safe. If the sizes are the same, compares all points
            int smallest_size = (cluster_gold->size <= cluster_out->size) ? cluster_gold->size : cluster_out->size;
            for (int p = 0; p < smallest_size; ++p) {
                if (cluster_out->points[p] != cluster_gold->points[p]) {
                    printf("SDC: [L:%d, N:%d, P:%d] Point -> OUT: %d , GOLD: %d\n",
                           level, cluster_count, p, cluster_out->points[p], cluster_gold->points[p]);
                    ++errors;
                }
            }

            //proceed to next clusters
            cluster_out = cluster_out->next_cluster;
            cluster_gold = cluster_gold->next_cluster;
            ++cluster_count;

            //detect different number of clusters in the level
            clusters_are_not_nullptr = (cluster_out != nullptr) && (cluster_gold != nullptr);
            if (!clusters_are_not_nullptr) {
                if ((cluster_out != nullptr) && (cluster_gold == nullptr)){
                    printf("SDC: [L:%d] Cluster count -> OUT: >=%d , GOLD: %d\n",
                           level, levels_out, levels_gold);
                    ++errors;
                }
                if ((cluster_out == nullptr) && (cluster_gold != nullptr)) {
                    printf("SDC: [L:%d] Cluster count -> OUT: %d , GOLD: >=%d\n",
                           level, levels_out, levels_gold);
                    ++errors;
                }
            }
        }
        ++iter_out;
        ++iter_gold;
        ++level;
    }

    return errors;
}

void usage(char *argv0) {
    const char *help =
            "Usage: %s [switches] \n"
                    "       -o output-filename     		: file containing the output result\n"
                    "       -g gold-filename     		: file containing the gold result\n";
    fprintf(stderr, help, argv0);
    exit(-1);
}

int main(int argc, char **argv) {
    printf("\n\n^^^^^^^^^^^^^^^^^^^^^^^^^ START OF DIANA GOLD CHECK ^^^^^^^^^^^^^^^^^^^^^^^^^\n");

    int     opt;
    char   *out_filename = 0;
    char   *gold_filename = 0;

    double  timing;

    while ( (opt=getopt(argc,argv,"o:g:?"))!= EOF) {
        switch (opt) {
            case 'o':
                out_filename=optarg;
                break;
            case 'g':
                gold_filename=optarg;
                break;
            case '?':
                usage(argv[0]);
                break;
            default:
                usage(argv[0]);
                break;
        }
    }


    if (out_filename == 0 || gold_filename == 0) usage(argv[0]);

    //printf("#HEADER filename:%s threshold:%f clusters:%d threads:%d\n",filename,threshold,nclusters,num_omp_threads);

    timing = omp_get_wtime();
    int sdcs = compare_out_and_gold(out_filename, gold_filename);
    timing = omp_get_wtime() - timing;

    printf("Time for comparing: %f\n", timing);
    printf("SDCs: %d\n", sdcs);

    printf("\n^^^^^^^^^^^^^^^^^^^^^^^^^ END OF DIANA GOLD CHECK ^^^^^^^^^^^^^^^^^^^^^^^^^\n");
    return sdcs;
}