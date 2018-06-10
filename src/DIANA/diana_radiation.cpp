//
// Created by gabriel on 6/10/18.
//

#include <getopt.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <iostream>
#include <fstream>
#include "dendrogram.h"
#include "diana.h"

#define MAX_ITER 2

typedef struct input_data {
    float **attributes;
    int num_objects;
    int num_attributes;
} input_data_t;

void dianaKernel(input_data_t *inputData, float kmeans_threshold) {
    initialize_cluster_ids();
    {
        srand(7);
        /* perform DIANA and saves result in dendrogram (cpp interface) */
        diana_clustering(inputData->attributes,
                         inputData->num_attributes,
                         inputData->num_objects,
                         kmeans_threshold);
    }
}

void usage(char *argv0) {
    const char *help =
            "Usage: %s [switches]\n"
                    "       -i input-filename   : file containing data to be clustered\n"
                    "       -o output-filename  : file containing the output result\n"
                    "       -g golden-filename  : file containing the golden result"
                    "       -t threshold		: kmeans threshold value\n"
                    "       -n no. of threads	: number of threads\n";
    fprintf(stderr, help, argv0);
    exit(-1);
}

input_data_t *getInputData(char *filename) {
    auto *inputData = (input_data_t *) (malloc(sizeof(input_data_t)));

    float *buf;
    float **attributes;
    char line[1024];
    int i, j;

    FILE *infile;
    if ((infile = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "Error: no such file (%s)\n", filename);
        exit(1);
    }
    while (fgets(line, 1024, infile) != NULL)
        if (strtok(line, " \t\n") != 0)
            inputData->num_objects++;
    rewind(infile);
    while (fgets(line, 1024, infile) != NULL) {
        if (strtok(line, " \t\n") != 0) {
            /* ignore the id (first attribute): numAttributes = 1; */
            while (strtok(NULL, " ,\t\n") != NULL) inputData->num_attributes++;
            break;
        }
    }

    /* allocate space for attributes[] and read attributes of all objects */
    buf = (float *) malloc(inputData->num_objects * inputData->num_attributes * sizeof(float));
    attributes = (float **) malloc(inputData->num_objects * sizeof(float *));
    attributes[0] = (float *) malloc(inputData->num_objects * inputData->num_attributes * sizeof(float));
    for (i = 1; i < inputData->num_objects; i++)
        attributes[i] = attributes[i - 1] + inputData->num_attributes;
    rewind(infile);
    i = 0;
    while (fgets(line, 1024, infile) != NULL) {
        if (strtok(line, " \t\n") == NULL) continue;
        for (j = 0; j < inputData->num_attributes; j++) {
            buf[i] = atof(strtok(NULL, " ,\t\n"));
            i++;
        }
    }
    fclose(infile);

    memcpy(attributes[0], buf, inputData->num_objects * inputData->num_attributes * sizeof(float));

    inputData->attributes = attributes;

    return inputData;
}

void flagIfDifferent(int a, int b, int level, int cluster_count, int *errors, char *description) {
    if (a != b) {
        printf("SDC: [L:%d, N:%d] %s -> OUT: %d , GOLD: %d\n",
               level, cluster_count, description, a, a);
        ++(*errors);
    }
}

int compare_out_and_gold(std::map<int, cluster_t*> *dendrogram_out, std::map<int, cluster_t*> *dendrogram_gold) {
    int errors = 0;

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


/**
 * High level wrapper that loops diana kernel, comparison and logging
 */
int main(int argc, char **argv) {
    int opt;
    char *input_filename = nullptr;
    char *output_filename = nullptr;
    char *golden_filename = nullptr;
    float kmeans_threshold = 0.001;
    int num_omp_threads = 1;

    while ((opt = getopt(argc, argv, "i:o:g:t:n:?")) != EOF) {
        switch (opt) {
            case 'i':
                input_filename = optarg;
                break;
            case 'o':
                output_filename = optarg;
                break;
            case 'g':
                golden_filename = optarg;
                break;
            case 't':
                kmeans_threshold = atof(optarg);
                break;
            case 'n':
                num_omp_threads = atoi(optarg);
                break;
            case '?':
                usage(argv[0]);
                break;
            default:
                usage(argv[0]);
                break;
        }
    }

    //TIMING setup_start
    //read data to input data struct
    input_data_t *input_data = getInputData(input_filename);
    //read gold file to golden_dendrogram
    std::map<int, cluster_t*> *golden_dendrogram = dendrogramFromBinaryFile(golden_filename);
    //TIMING setup_end
    
    //loop indefinitely
    for (int i = 0; i < MAX_ITER; ++i) {
        //TIMING loop_start

        //TIMING kernel_start
        //LOGS iteration_start
        dianaKernel(input_data, kmeans_threshold);
        //LOGS iteration_end
        //TIMING kernel_end

        //TIMING check_start
        //compare out with gold (output_dendrogram, golden_dendrogram) --> adapt to use log_error_detail(error_detail)
        int detected_sdcs = compare_out_and_gold(getOutputDendrogram(), golden_dendrogram);
        std::cout << "Detected " << detected_sdcs << " SDCS\n";
        //LOG log_error_count(detected_sdcs)
        //TIMING check_end

        if (detected_sdcs > 0) {
            //re-read input and gold (they may be corrupted)
            input_data= getInputData(input_filename);
            golden_dendrogram= dendrogramFromBinaryFile(golden_filename);
        }

        //TIMING loop_end

        //TIMING print all times
    }

    //LOGS end_log_file()
}