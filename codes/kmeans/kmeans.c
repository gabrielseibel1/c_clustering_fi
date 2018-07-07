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
/**   File:         example.c                                           **/
/**   Description:  Takes as input a file:                              **/
/**                 ascii  file: containing 1 data point per line       **/
/**                 binary file: first int is the number of objects     **/
/**                              2nd int is the no. of features of each **/
/**                              object                                 **/
/**                 This example performs a fuzzy c-means clustering    **/
/**                 on the data. Fuzzy clustering is performed using    **/
/**                 min to max clusters and the clustering that gets    **/
/**                 the best score according to a compactness and       **/
/**                 separation criterion are returned.                  **/
/**   Author:  Wei-keng Liao                                            **/
/**            ECE Department Northwestern University                   **/
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
/*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <omp.h>
#include "getopt.h"

#include "kmeans.h"

extern double wtime(void);

int num_omp_threads = 1;
int mark = 0;

/*---< usage() >------------------------------------------------------------*/
void usage(char *argv0) {
    char *help =
        "Usage: %s [switches] -i filename\n"
        "       -i filename     		: file containing data to be clustered\n"
        "       -o output-filename     		: file containing the output result\n"
        "       -b                 	: input file is in binary format\n"
        "       -k                 	: number of clusters (default is 5) \n"
        "       -t threshold		: threshold value\n"
        "       -n no. of threads	: number of threads";
    fprintf(stderr, help, argv0);
    exit(-1);
}

int equality_check(int var, int var2){
  if(var == var2)
    return 1;
  else
    return 0;
}

void inc_cont_SDC_file_and_loop(){
  FILE *f = fopen("SDC_count.txt", "r");
    if (f == NULL){
      printf("Arquivo nao foi criado ainda!\n");
    }
	  else {
		fscanf(f,"%d",&mark);
	  	fclose(f);
	  }

	  mark++;
	  printf("Deu SDC!\n");
	  f = fopen("SDC_count.txt", "w");
    if (f == NULL){
      printf("Error opening file!\n");
      exit(1);
    }
  fprintf(f, "%d", mark);
  fclose(f);

  while (1) {
    printf("Infinity Loop\n");
  }
}

/*---< main() >-------------------------------------------------------------*/
int main(int argc, char **argv) {
    int     opt;
    extern char   *optarg;
    extern int     optind;
    int     nclusters=5;
    char   *filename = 0;
    char   *out_filename = 0;
    float  *buf;
    float **attributes;
    float **cluster_centres=NULL;
    int     i, j;

    int     numAttributes;
    int     numAttributes_duplicated;
    int     numObjects;
    char    line[1024];
    int     isBinaryFile = 0;
    int     nloops = 1;
    float   threshold = 0.001;
    double  timing;

    while ( (opt=getopt(argc,argv,"i:o:k:t:b:n:?"))!= EOF) {
        switch (opt) {
        case 'i':
            filename=optarg;
            break;
        case 'o':
            out_filename=optarg;
            break;
        case 'b':
            isBinaryFile = 1;
            break;
        case 't':
            threshold=atof(optarg);
            break;
        case 'k':
            nclusters = atoi(optarg);
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


    if (filename == 0 || out_filename == 0) usage(argv[0]);

    numAttributes = numObjects = 0;
    numAttributes_duplicated = 0;

    /* from the input file, get the numAttributes and numObjects ------------*/

    if (isBinaryFile) {
        int infile;
        if ((infile = open(filename, O_RDONLY, "0600")) == -1) {
            fprintf(stderr, "Error: no such file (%s)\n", filename);
            exit(1);
        }
        read(infile, &numObjects,    sizeof(int));
        read(infile, &numAttributes, sizeof(int));


        /* allocate space for attributes[] and read attributes of all objects */
        buf           = (float*) malloc(numObjects*numAttributes*sizeof(float));
        attributes    = (float**)malloc(numObjects*sizeof(float*));
        attributes[0] = (float*) malloc(numObjects*numAttributes*sizeof(float));
        for (i=1; i<numObjects; i++)
            attributes[i] = attributes[i-1] + numAttributes;

        read(infile, buf, numObjects*numAttributes*sizeof(float));

        close(infile);
    }
    else {
        FILE *infile;
        if ((infile = fopen(filename, "r")) == NULL) {
            fprintf(stderr, "Error: no such file (%s)\n", filename);
            exit(1);
        }
        while (fgets(line, 1024, infile) != NULL)
            if (strtok(line, " \t\n") != 0)
                numObjects++;
        rewind(infile);
        while (fgets(line, 1024, infile) != NULL) {
            if (strtok(line, " \t\n") != 0) {
                while (strtok(NULL, " ,\t\n") != NULL) numAttributes++;
                break;
            }
        }
        rewind(infile);
        while (fgets(line, 1024, infile) != NULL) {
            if (strtok(line, " \t\n") != 0) {
                while (strtok(NULL, " ,\t\n") != NULL) numAttributes_duplicated++;
                break;
            }
        }


        /* allocate space for attributes[] and read attributes of all objects */

        if(equality_check(numAttributes, numAttributes_duplicated) == 1){
          buf = (float*) malloc(numObjects*numAttributes*sizeof(float));
        } else {
            inc_cont_SDC_file_and_loop();
        }
        attributes    = (float**)malloc(numObjects*sizeof(float*));

        if(equality_check(numAttributes, numAttributes_duplicated) == 1){
          attributes[0] = (float*) malloc(numObjects*numAttributes*sizeof(float));
        } else {
            inc_cont_SDC_file_and_loop();
        }

        if(equality_check(numAttributes, numAttributes_duplicated) == 1){
          for (i=1; i<numObjects; i++)
              attributes[i] = attributes[i-1] + numAttributes;
        } else {
            inc_cont_SDC_file_and_loop();
        }

        rewind(infile);
        i = 0;
        while (fgets(line, 1024, infile) != NULL) {
            if (strtok(line, " \t\n") == NULL) continue;
            for (j=0; j<numAttributes; j++) {
              if(equality_check(numAttributes, numAttributes_duplicated) == 1){
                buf[i] = atof(strtok(NULL, " ,\t\n"));
                i++;
              } else {
                  inc_cont_SDC_file_and_loop();
              }
            }
        }
    }
    printf("I/O completed\n");

    if(equality_check(numAttributes, numAttributes_duplicated) == 1){
      memcpy(attributes[0], buf, numObjects*numAttributes*sizeof(float));
    } else {
        inc_cont_SDC_file_and_loop();
    }

    timing = omp_get_wtime();
    for (i=0; i<nloops; i++) {

        cluster_centres = NULL;
        if(equality_check(numAttributes, numAttributes_duplicated) == 1){
          cluster(numObjects,numAttributes,attributes,nclusters,threshold,&cluster_centres);
        } else {
            inc_cont_SDC_file_and_loop();
        }

    }
    timing = omp_get_wtime() - timing;


    printf("number of Clusters %d\n",nclusters);
    printf("number of Attributes %d\n\n",numAttributes);
    printf("number of attributes_duplicated %d\n\n",numAttributes_duplicated);
    printf("number of Objects %d\n\n",numObjects);

    FILE *file;
    if( (file = fopen(out_filename, "wb" )) == 0 )
        printf( "The GOLD file was not opened\n" );
    for (i=0; i< nclusters; i++) {
        fwrite(&i, 1, sizeof(int), file);
        for (j=0; j<numAttributes; j++){
          if(equality_check(numAttributes, numAttributes_duplicated) == 1){
            fwrite(&cluster_centres[i][j], 1, sizeof(float), file);
          } else {
              inc_cont_SDC_file_and_loop();
          }
        }
    }

    fclose(file);

    printf("Time for process: %f\n", timing);

    free(attributes);
    free(cluster_centres[0]);
    free(cluster_centres);
    free(buf);
    return(0);
}
