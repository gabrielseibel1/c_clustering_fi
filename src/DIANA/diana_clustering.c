/*****************************************************************************/
/*IMPORTANT:  READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.         */
/*By downloading, copying, installing or using the software you agree        */
/*to this license.  If you do not agree to this license, do not download,    */
/*install, copy or use the software.                                         */
/*                                                                           */
/*                                                                           */
/*Copyright (c) 2005 Northwestern University                                 */
/*All rights reserved.                                                       */

/*Redistribution of the software in source and binary forms,                 */
/*with or without modification, is permitted provided that the               */
/*following conditions are met:                                              */
/*                                                                           */
/*1       Redistributions of source code must retain the above copyright     */
/*        notice, this list of conditions and the following disclaimer.      */
/*                                                                           */
/*2       Redistributions in binary form must reproduce the above copyright   */
/*        notice, this list of conditions and the following disclaimer in the */
/*        documentation and/or other materials provided with the distribution.*/
/*                                                                            */
/*3       Neither the name of Northwestern University nor the names of its    */
/*        contributors may be used to endorse or promote products derived     */
/*        from this software without specific prior written permission.       */
/*                                                                            */
/*THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS    */
/*IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED      */
/*TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT AND         */
/*FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL          */
/*NORTHWESTERN UNIVERSITY OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,       */
/*INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES          */
/*(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR          */
/*SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)          */
/*HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,         */
/*STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN    */
/*ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE             */
/*POSSIBILITY OF SUCH DAMAGE.                                                 */
/******************************************************************************/
/*************************************************************************/
/**   File:         kmeans_clustering.c                                 **/
/**   Description:  Implementation of regular k-means clustering        **/
/**                 algorithm                                           **/
/**   Author:  Wei-keng Liao                                            **/
/**            ECE Department, Northwestern University                  **/
/**            email: wkliao@ece.northwestern.edu                       **/
/**                                                                     **/
/**   Edited by: Jay Pisharath                                          **/
/**              Northwestern University.                               **/
/**                                                                     **/
/**   ================================================================  **/
/**																		**/
/**   Edited by: Sang-Ha  Lee											**/
/**				 University of Virginia									**/
/**																		**/
/**   Description:	No longer supports fuzzy c-means clustering;	 	**/
/**					only regular k-means clustering.					**/
/**					Simplified for main functionality: regular k-means	**/
/**					clustering.											**/
/**                                                                     **/
/**   ================================================================  **/
/**																		**/
/**   Edited by: Gabriel de Souza Seibel    							**/
/**				 UFRGS - Universidade Federal do Rio Grande do Sul		**/
/**																		**/
/**   Description:	Changed k-means clustering to divisive analysis     **/
/**					hierarchical clustering (DIANA).                    **/
/**                                                                     **/
/*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include "diana.h"
#include "../kmeans/kmeans.h"
#include "../kmeans/kmeans_clustering.h"
#include <stdbool.h>

#define RANDOM_MAX 2147483647

#ifndef FLT_MAX
#define FLT_MAX 3.40282347e+38
#endif

extern double wtime(void);

extern int num_omp_threads;

cluster_t *new_father_cluster(int size) {
    cluster_t *father_cluster = (cluster_t *) malloc(sizeof(cluster_t));
    father_cluster->size = size;
    father_cluster->points = malloc(sizeof(int)*size);
    for (int point = 0; point < size; ++point) {
        father_cluster->points[point] = point;
    }
    father_cluster->father_cluster = NULL; //there is none
    father_cluster->next_cluster = NULL; //there is none
    father_cluster->left_child = NULL; //will be allocated later
    father_cluster->right_child = NULL; //will be allocated later

    return father_cluster;
}

int *membership_from_kmeans(float **points, int n_features, int n_points, int k, float threshold) {
    int *membership;

    membership = (int *) malloc(n_points * sizeof(int));

    srand(7);
    /* perform regular Kmeans */
    kmeans_clustering(points,
                      n_features,
                      n_points,
                      k,
                      threshold,
                      membership);

    return membership;
}

/*----< diana_clustering() >---------------------------------------------*/
cluster_t *diana_clustering(float **all_points,    /* in: [n_points][n_features] */
                            int n_features, int n_points, float threshold) {


    //create first cluster (with all points) and initialize dendrogram with it
    cluster_t *father_cluster = new_father_cluster(n_points);
    initialize_dendrogram(father_cluster);

    //iterate over the levels of the dendrogram while not all clusters are unitary
    bool there_was_a_cluster_split; //condition to stop algorithm
    int level = 1; //1 (not 0) because father cluster has already been inserted
    do {
        there_was_a_cluster_split = false;
        int n_clusters_in_anterior_level = (int) pow(2, level-1); // 2^lvl-1

        //for each big cluster (of the anterior level) build two smaller clusters
        for (int cluster_to_divide_index = 0; cluster_to_divide_index < n_clusters_in_anterior_level; ++cluster_to_divide_index) {

            cluster_t* cluster_to_divide = get_cluster(level - 1, cluster_to_divide_index);

            if (cluster_to_divide && cluster_to_divide->size > 1) { //no need to split a cluster that has only one element

                //retrieve attributes from each point of the cluster to be split
                float** points_with_attributes_from_cluster_to_divide = get_points_in_cluster(cluster_to_divide, all_points, n_features);
                //get list of points that belong to new clusters
                int *points_membership = membership_from_kmeans(points_with_attributes_from_cluster_to_divide,
                                                                n_features,
                                                                cluster_to_divide->size,
                                                                2, /*split in two new clusters*/
                                                                threshold);

                there_was_a_cluster_split = split_cluster(level, points_membership, cluster_to_divide) || there_was_a_cluster_split;
            }
        }

        level = get_levels();

        //print_dendrogram();
    } while (/*ORDER OF OPERANDS HERE IS IMPORTANT!*/there_was_a_cluster_split/* && !are_all_clusters_in_level_unitary(level)*/);

    return father_cluster;
}

