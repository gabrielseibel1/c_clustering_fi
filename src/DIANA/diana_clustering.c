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
#include <stdbool.h>

#define RANDOM_MAX 2147483647

#ifndef FLT_MAX
#define FLT_MAX 3.40282347e+38
#endif

extern double wtime(void);

extern int num_omp_threads;

int find_nearest_point(float *pt,          /* [nfeatures] */
                       int nfeatures,
                       float **pts,         /* [npts][nfeatures] */
                       int npts) {
    int index, i;
    float min_dist = FLT_MAX;

    /* find the cluster center id with min distance to pt */
    for (i = 0; i < npts; i++) {
        float dist;
        dist = euclid_dist_2(pt, pts[i], nfeatures);  /* no need square root */
        if (dist < min_dist) {
            min_dist = dist;
            index = i;
        }
    }
    return (index);
}

/*----< euclid_dist_2() >----------------------------------------------------*/
/* multi-dimensional spatial Euclid distance square */
__inline
float euclid_dist_2(float *pt1,
                    float *pt2,
                    int numdims) {
    int i;
    float ans = 0.0;

    for (i = 0; i < numdims; i++)
        ans += (pt1[i] - pt2[i]) * (pt1[i] - pt2[i]);

    return (ans);
}

cluster_t *new_father_cluster(int size) {
    cluster_t *father_cluster = (cluster_t *) malloc(sizeof(cluster_t));
    father_cluster->size = size;
    for (int point = 0; point < size; ++point) {
        father_cluster->points[point] = point;
    }
    father_cluster->next_cluster = NULL;
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
cluster_t *diana_clustering(float **points,    /* in: [n_points][n_features] */
                            int n_features, int n_points) {

    cluster_t *father_cluster = new_father_cluster(n_points);

    //iterate over the levels of the dendrogram while not all clusters are unitary
    bool all_clusters_in_level_are_unitary = (n_points == 1);
    int level = 0; reset_levels();
    do {

        int n_clusters_in_level = (int) pow(2, level); // 2^lvl

        //for each big cluster (of the anterior level) build two smaller clusters
        for (int cluster_to_divide_index = 0;
             cluster_to_divide_index < n_clusters_in_level; ++cluster_to_divide_index) {

            for (int i = 0; i < 2; ++i) {
                int n_points_in_cluster_to_divide;
                float **points_in_cluster_to_divide = get_points_in_cluster(level,
                                                                            cluster_to_divide_index,
                                                                            points,
                                                                            n_features,
                                                                            &n_points_in_cluster_to_divide);

                //get list of points that belong to new cluster
                int *points_membership = membership_from_kmeans(points_in_cluster_to_divide,
                                                                n_features,
                                                                n_points_in_cluster_to_divide,
                                                                2, /*split in two new clusters*/
                                                                0.001 /*get from user*/ );

                insert_2_clusters_in_dendrogram(level, points_membership, n_points_in_cluster_to_divide);
            }
        }
        ++level; inc_levels();
        all_clusters_in_level_are_unitary = (bool) are_all_clusters_in_level_unitary(level);
    } while (!all_clusters_in_level_are_unitary);
}

