/*
   * Code based on Github repository from UFRGS-CAROL/radiation-benchmarks
   * used only for achademic purposes
   * 
   * 
   *
   * Edited by Geronimo Veit, UFRGS - June, 2018
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>

#ifdef LOGS
#include "../../include/log_helper.h"
#endif

#ifdef TIMING
long long timing_get_time() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000000) + tv.tv_usec;
}

long long setup_start, setup_end;
long long loop_start, loop_end;
long long kernel_start, kernel_end;
long long check_start, check_end;
#endif

/* Global Variables */

FILE *out;
char outputFile[200];

//////////////////////

/* This area is specific for kernel activies */




#define ERROR -1
#define SUCCESS 0

#define NOT_USED  0 /* node is currently not used */
#define LEAF_NODE 1 /* node contains a leaf node */
#define A_MERGER  2 /* node contains a merged pair of root clusters */
#define MAX_LABEL_LEN 16

#define AVERAGE_LINKAGE  'a' /* choose average distance */
#define CENTROID_LINKAGE 't' /* choose distance between cluster centroids */
#define COMPLETE_LINKAGE 'c' /* choose maximum distance */
#define SINGLE_LINKAGE   's' /* choose minimum distance */

#define alloc_mem(N, T) (T *) calloc(N, sizeof(T))
#define alloc_fail(M) fprintf(stderr,                                   \
                              "Failed to allocate memory for %s.\n", M)
#define read_fail(M) fprintf(stderr, "Failed to read %s from file.\n", M)
#define invalid_node(I) fprintf(stderr,                                 \
                                "Invalid cluster node at index %d.\n", I)

typedef struct cluster_s cluster_t;
typedef struct cluster_node_s cluster_node_t;
typedef struct neighbour_s neighbour_t;
typedef struct item_s item_t;

float (*distance_fptr)(float **, const int *, const int *, int, int);

typedef struct coord_s {
        float x, y;
} coord_t;

struct cluster_s {
        int num_items; /* number of items that was clustered */
        int num_clusters; /* current number of root clusters */
        int num_nodes; /* number of leaf and merged clusters */
        cluster_node_t *nodes; /* leaf and merged clusters */
        float **distances; /* distance between leaves */
};

struct cluster_node_s {
        int type; /* type of the cluster node */
        int is_root; /* true if cluster hasn't merged with another */
        int height; /* height of node from the bottom */
        coord_t centroid; /* centroid of this cluster */
        char *label; /* label of a leaf node */
        int *merged; /* indexes of root clusters merged */
        int num_items; /* number of leaf nodes inside new cluster */
        int *items; /* array of leaf nodes indices inside merged clusters */
        neighbour_t *neighbours; /* sorted linked list of distances to roots */
};

struct neighbour_s {
        int target; /* the index of cluster node representing neighbour */
        float distance; /* distance between the nodes */
        neighbour_t *next, *prev; /* linked list entries */
};

struct item_s {
        coord_t coord; /* coordinate of the input data point */
        char label[MAX_LABEL_LEN]; /* label of the input data point */
};

float euclidean_distance(const coord_t *a, const coord_t *b)
{
        return sqrt(pow(a->x - b->x, 2) + pow(a->y - b->y, 2));
}

void fill_euclidean_distances(float **matrix, int num_items,
                              const item_t items[])
{
        for (int i = 0; i < num_items; ++i)
                for (int j = 0; j < num_items; ++j) {
                        matrix[i][j] =
                                euclidean_distance(&(items[i].coord),
                                                   &(items[j].coord));
                        matrix[j][i] = matrix[i][j];
                }
}

float **generate_distance_matrix(int num_items, const item_t items[])
{
        float **matrix = alloc_mem(num_items, float *);
        if (matrix) {
                for (int i = 0; i < num_items; ++i) {
                        matrix[i] = alloc_mem(num_items, float);
                        if (!matrix[i]) {
                                alloc_fail("distance matrix row");
                                num_items = i;
                                for (i = 0; i < num_items; ++i)
                                        free(matrix[i]);
                                free(matrix);
                                matrix = NULL;
                                break;
                        }
                }
                if (matrix)
                        fill_euclidean_distances(matrix, num_items, items);
        } else
                alloc_fail("distance matrix");
        return matrix;
}

float single_linkage(float **distances, const int a[],
                     const int b[], int m, int n)
{
        float min = FLT_MAX, d;
        for (int i = 0; i < m; ++i)
                for (int j = 0; j < n; ++j) {
                        d = distances[a[i]][b[j]];
                        if (d < min)
                                min = d;
                }
        return min;
}

float complete_linkage(float **distances, const int a[],
                       const int b[], int m, int n)
{
        float d, max = 0.0 /* assuming distances are positive */;
        for (int i = 0; i < m; ++i)
                for (int j = 0; j < n; ++j) {
                        d = distances[a[i]][b[j]];
                        if (d > max)
                                max = d;
                }
        return max;
}

float average_linkage(float **distances, const int a[],
                      const int b[], int m, int n)
{
        float total = 0.0;
        for (int i = 0; i < m; ++i)
                for (int j = 0; j < n; ++j)
                        total += distances[a[i]][b[j]];
        return total / (m * n);
}

float centroid_linkage(float **distances, const int a[],
                       const int b[], int m, int n)
{
        return 0; /* empty function */
}

float get_distance(cluster_t *cluster, int index, int target)
{
        /* if both are leaves, just use the distances matrix */
        if (index < cluster->num_items && target < cluster->num_items)
                return cluster->distances[index][target];
        else {
                cluster_node_t *a = &(cluster->nodes[index]);
                cluster_node_t *b = &(cluster->nodes[target]);
                if (distance_fptr == centroid_linkage)
                        return euclidean_distance(&(a->centroid),
                                                  &(b->centroid));
                else return distance_fptr(cluster->distances,
                                          a->items, b->items,
                                          a->num_items, b->num_items);
        }
}

void free_neighbours(neighbour_t *node)
{
        neighbour_t *t;
        while (node) {
                t = node->next;
                free(node);
                node = t;
        }
}

void free_cluster_nodes(cluster_t *cluster)
{
        for (int i = 0; i < cluster->num_nodes; ++i) {
                cluster_node_t *node = &(cluster->nodes[i]);
                if (node->label)
                        free(node->label);
                if (node->merged)
                        free(node->merged);
                if (node->items)
                        free(node->items);
                if (node->neighbours)
                        free_neighbours(node->neighbours);
        }
        free(cluster->nodes);
}

void free_cluster(cluster_t * cluster)
{
        if (cluster) {
                if (cluster->nodes)
                        free_cluster_nodes(cluster);
                if (cluster->distances) {
                        for (int i = 0; i < cluster->num_items; ++i)
                                free(cluster->distances[i]);
                        free(cluster->distances);
                }
                free(cluster);
        }
}

void insert_before(neighbour_t *current, neighbour_t *neighbours,
                   cluster_node_t *node)
{
        neighbours->next = current;
        if (current->prev) {
                current->prev->next = neighbours;
                neighbours->prev = current->prev;
        } else
                node->neighbours = neighbours;
        current->prev = neighbours;
}

void insert_after(neighbour_t *current, neighbour_t *neighbours)
{
        neighbours->prev = current;
        current->next = neighbours;
}

void insert_sorted(cluster_node_t *node, neighbour_t *neighbours)
{
        neighbour_t *temp = node->neighbours;
        while (temp->next) {
                if (temp->distance >= neighbours->distance) {
                        insert_before(temp, neighbours, node);
                        return;
                }
                temp = temp->next;
        }
        if (neighbours->distance < temp->distance)
                insert_before(temp, neighbours, node);
        else
                insert_after(temp, neighbours);
}


neighbour_t *add_neighbour(cluster_t *cluster, int index, int target)
{
        neighbour_t *neighbour = alloc_mem(1, neighbour_t);
        if (neighbour) {
                neighbour->target = target;
                neighbour->distance = get_distance(cluster, index, target);
                cluster_node_t *node = &(cluster->nodes[index]);
                if (node->neighbours)
                        insert_sorted(node, neighbour);
                else
                        node->neighbours = neighbour;
        } else
                alloc_fail("neighbour node");
        return neighbour;
}

cluster_t *update_neighbours(cluster_t *cluster, int index)
{
        cluster_node_t *node = &(cluster->nodes[index]);
        if (node->type == NOT_USED) {
                invalid_node(index);
                cluster = NULL;
        } else {
                int root_clusters_seen = 1, target = index;
                while (root_clusters_seen < cluster->num_clusters) {
                        cluster_node_t *temp = &(cluster->nodes[--target]);
                        if (temp->type == NOT_USED) {
                                invalid_node(index);
                                cluster = NULL;
                                break;
                        }
                        if (temp->is_root) {
                                ++root_clusters_seen;
                                add_neighbour(cluster, index, target);
                        }
                }
        }
        return cluster;
}

#define init_leaf(cluster, node, item, len)             \
        do {                                            \
                strncpy(node->label, item->label, len); \
                node->centroid = item->coord;           \
                node->type = LEAF_NODE;                 \
                node->is_root = 1;                      \
                node->height = 0;                       \
                node->num_items = 1;                    \
                node->items[0] = cluster->num_nodes++;  \
        } while (0)                                     \

cluster_node_t *add_leaf(cluster_t *cluster, const item_t *item)
{
        cluster_node_t *leaf = &(cluster->nodes[cluster->num_nodes]);
        int len = strlen(item->label) + 1;
        leaf->label = alloc_mem(len, char);
        if (leaf->label) {
                leaf->items = alloc_mem(1, int);
                if (leaf->items) {
                        init_leaf(cluster, leaf, item, len);
                        cluster->num_clusters++;
                } else {
                        alloc_fail("node items");
                        free(leaf->label);
                        leaf = NULL;
                }
        } else {
                alloc_fail("node label");
                leaf = NULL;
        }
        return leaf;
}

#undef init_leaf

cluster_t *add_leaves(cluster_t *cluster, item_t *items)
{
        for (int i = 0; i < cluster->num_items; ++i) {
                if (add_leaf(cluster, &items[i]))
                        update_neighbours(cluster, i);
                else {
                        cluster = NULL;
                        break;
                }
        }
        return cluster;
}

void print_cluster_items(cluster_t *cluster, int index)
{
        cluster_node_t *node = &(cluster->nodes[index]);
        
        fprintf(out, "\n");
}

void print_cluster_node(cluster_t *cluster, int index)
{
        cluster_node_t *node = &(cluster->nodes[index]);

        fprintf(out, "Node %d - height: %d, centroid: (%5.3f, %5.3f)\n",
                index, node->height, node->centroid.x, node->centroid.y);

        print_cluster_items(cluster, index);
        neighbour_t *t = node->neighbours;
        
        fprintf(out, "\n");
}

void merge_items(cluster_t *cluster, cluster_node_t *node,
                 cluster_node_t **to_merge)
{
        node->type = A_MERGER;
        node->is_root = 1;
        node->height = -1;

        /* copy leaf indexes from merged clusters */
        int k = 0, idx;
        coord_t centroid = { .x = 0.0, .y = 0.0 };
        for (int i = 0; i < 2; ++i) {
                cluster_node_t *t = to_merge[i];
                t->is_root = 0; /* no longer root: merged */
                if (node->height == -1 ||
                    node->height < t->height)
                        node->height = t->height;
                for (int j = 0; j < t->num_items; ++j) {
                        idx = t->items[j];
                        node->items[k++] = idx;
                }
                centroid.x += t->num_items * t->centroid.x;
                centroid.y += t->num_items * t->centroid.y;
        }
        /* calculate centroid */
        node->centroid.x = centroid.x / k;
        node->centroid.y = centroid.y / k;
        node->height++;
}

#define merge_to_one(cluster, to_merge, node, node_idx)         \
        do {                                                    \
                node->num_items = to_merge[0]->num_items +      \
                        to_merge[1]->num_items;                 \
                node->items = alloc_mem(node->num_items, int);  \
                if (node->items) {                              \
                        merge_items(cluster, node, to_merge);   \
                        cluster->num_nodes++;                   \
                        cluster->num_clusters--;                \
                        update_neighbours(cluster, node_idx);   \
                } else {                                        \
                        alloc_fail("array of merged items");    \
                        free(node->merged);                     \
                        node = NULL;                            \
                }                                               \
        } while(0)                                              \

cluster_node_t *merge(cluster_t *cluster, int first, int second)
{
        int new_idx = cluster->num_nodes;
        cluster_node_t *node = &(cluster->nodes[new_idx]);
        node->merged = alloc_mem(2, int);
        if (node->merged) {
                cluster_node_t *to_merge[2] = {
                        &(cluster->nodes[first]),
                        &(cluster->nodes[second])
                };
                node->merged[0] = first;
                node->merged[1] = second;
                merge_to_one(cluster, to_merge, node, new_idx);
        } else {
                alloc_fail("array of merged nodes");
                node = NULL;
        }
        return node;
}

#undef merge_to_one

void find_best_distance_neighbour(cluster_node_t *nodes,
                                  int node_idx,
                                  neighbour_t *neighbour,
                                  float *best_distance,
                                  int *first, int *second)
{
        while (neighbour) {
                if (nodes[neighbour->target].is_root) {
                        if (*first == -1 ||
                            neighbour->distance < *best_distance) {
                                *first = node_idx;
                                *second = neighbour->target;
                                *best_distance = neighbour->distance;
                        }
                        break;
                }
                neighbour = neighbour->next;
        }
}


int find_clusters_to_merge(cluster_t *cluster, int *first, int *second)
{
        float best_distance = 0.0;
        int root_clusters_seen = 0;
        int j = cluster->num_nodes; /* traverse hierarchy top-down */
        *first = -1;
        while (root_clusters_seen < cluster->num_clusters) {
                cluster_node_t *node = &(cluster->nodes[--j]);
                if (node->type == NOT_USED || !node->is_root)
                        continue;
                ++root_clusters_seen;
                find_best_distance_neighbour(cluster->nodes, j,
                                             node->neighbours,
                                             &best_distance,
                                             first, second);
        }
        return *first;
}

cluster_t *merge_clusters(cluster_t *cluster)
{
        int first, second;
        while (cluster->num_clusters > 1) {
                if (find_clusters_to_merge(cluster, &first, &second) != -1)
                        merge(cluster, first, second);
        }
        return cluster;
}

#define init_cluster(cluster, num_items, items)                         \
        do {                                                            \
                cluster->distances =                                    \
                        generate_distance_matrix(num_items, items);     \
                if (!cluster->distances)                                \
                        goto cleanup;                                   \
                cluster->num_items = num_items;                         \
                cluster->num_nodes = 0;                                 \
                cluster->num_clusters = 0;                              \
                if (add_leaves(cluster, items))                         \
                        merge_clusters(cluster);                        \
                else                                                    \
                        goto cleanup;                                   \
        } while (0)                                                     \

cluster_t *agglomerate(int num_items, item_t *items)
{
        cluster_t *cluster = alloc_mem(1, cluster_t);
        if (cluster) {
                cluster->nodes = alloc_mem(2 * num_items - 1, cluster_node_t);
                if (cluster->nodes)
                        init_cluster(cluster, num_items, items);
                else {
                        alloc_fail("cluster nodes");
                        goto cleanup;
                }
        } else
                alloc_fail("cluster");
        goto done;

cleanup:
        free_cluster(cluster);
        cluster = NULL;

done:
        return cluster;
}

#undef init_cluster

int print_root_children(cluster_t *cluster, int i, int nodes_to_discard)
{
        cluster_node_t *node = &(cluster->nodes[i]);
        int roots_found = 0;
        if (node->type == A_MERGER) {
                for (int j = 0; j < 2; ++j) {
                        int t = node->merged[j];
                        if (t < nodes_to_discard) {
                                print_cluster_items(cluster, t);
                                ++roots_found;
                        }
                }
        }
        return roots_found;
}

void get_k_clusters(cluster_t *cluster, int k)
{
        if (k < 1)
                return;
        if (k > cluster->num_items)
                k = cluster->num_items;

        int i = cluster->num_nodes - 1;
        int roots_found = 0;
        int nodes_to_discard = cluster->num_nodes - k + 1;
        while (k) {
                if (i < nodes_to_discard) {
                        print_cluster_items(cluster, i);
                        roots_found = 1;
                } else
                        roots_found = print_root_children(cluster, i,
                                                          nodes_to_discard);
                k -= roots_found;
                --i;
        }
}

void print_cluster(cluster_t *cluster)
{
        for (int i = 0; i < cluster->num_nodes; ++i)
                print_cluster_node(cluster, i);
}

int read_items(int count, item_t *items, FILE *f)
{
        for (int i = 0; i < count; ++i) {
                item_t *t = &(items[i]);
                if (fscanf(f, "%[^|]| %10f %10f\n",
                           t->label, &(t->coord.x),
                           &(t->coord.y)))
                        continue;
                read_fail("item line");
                return i;
        }
        return count;
}

int read_items_from_file(item_t **items, FILE *f)
{
        int count, r;
        r = fscanf(f, "%5d\n", &count);
        if (r == 0) {
                read_fail("number of lines");
                return 0;
        }
        if (count) {
                *items = alloc_mem(count, item_t);
                if (*items) {
                        if (read_items(count, *items, f) != count)
                                free(items);
                } else
                        alloc_fail("items array");
        }
        return count;
}

void set_linkage(char linkage_type)
{
        switch (linkage_type) {
        case AVERAGE_LINKAGE:
                distance_fptr = average_linkage;
                break;
        case COMPLETE_LINKAGE:
                distance_fptr = complete_linkage;
                break;
        case CENTROID_LINKAGE:
                distance_fptr = centroid_linkage;
                break;
        case SINGLE_LINKAGE:
        default: distance_fptr = single_linkage;
        }
}

int process_input(item_t **items, const char *fname)
{
        int count = 0;
        FILE *f = fopen(fname, "r");
        if (f) {
                count = read_items_from_file(items, f);
                fclose(f);
        } else
                fprintf(stderr, "Failed to open input file %s.\n", fname);
        return count;
}





/* End of kernel area */


void init_agnes(char *input, char *nClusters, char *tLinkage) {

	char clus[5], link[10], entry[20];
	char link_type, *in;	
	
	strcpy(entry, input);
	strcpy(clus, nClusters);
	strcpy(link, tLinkage);
	in = strtok(entry, "/");
	in = strtok(NULL, "/");
	in = strtok(NULL, ".");

	if (strcmp(link, "single") == 0) {
		link_type = 's';
	}
	else if (strcmp(link, "average") == 0) {
		link_type = 'a';
	}
	else if (strcmp(link, "t") == 0) {
		link_type = 't';
	}
	else if (strcmp(link, "c") == 0) {
		link_type = 'c';
	}	
	
	snprintf(outputFile, 200, "OUTPUT%s_%s_%s.txt", clus, link, in);
        out = fopen(outputFile, "w");
	if (out == NULL) {
                fprintf(stderr, "Failed to open out file\n");
                exit(1); //kill execution
	}

	item_t *items = NULL;
        int num_items = process_input(&items, input);
        set_linkage(link_type);
        if (num_items) {
                cluster_t *cluster = agglomerate(num_items, items);
                free(items);

                if (cluster) {
                        fprintf(out, "CLUSTER HIERARCHY\n"
                                "--------------------\n");
                        print_cluster(cluster);

                        int k = atoi(clus);
                        fprintf(out, "\n\n%d CLUSTERS\n"
                                "--------------------\n", k);
                        get_k_clusters(cluster, k);
                        free_cluster(cluster);
                }
        }


        if (fclose(out) != SUCCESS) {

                fprintf(stderr, "Failed to close output file\n");
                exit(1);
        }
}

void changeInput(char *file) {
	
	int ch, loop = 0;
	FILE *ft;
	if (ft = fopen(file, "r+")) {
		while( ((ch = fgetc(ft)) != EOF) && (loop < 1000)) {
			if (ch == '1') {
				fseek(ft, -1, SEEK_CUR);
				fputc('2', ft);
				fseek(ft, 0, SEEK_CUR);
				loop++;
			}
		}
		fclose(ft);
	}
	else {
		fprintf(stderr, "Error in opening %s file for reading... exiting\n", file);
        	exit(1);
	}

}

int readTxtFile(char *inputFile) {

	int numElements = 0, carac;
	FILE *fin;

	if (fin = fopen(inputFile, "r")) {
		do {
			carac = fgetc(fin);
			if(feof(fin))
				break;
			numElements++;

		}while(1);
		fclose(fin);	
	}
	else {
        	fprintf(stderr, "Error in opening %s file for reading... exiting\n", inputFile);
        	return ERROR;
    	}
	
	return numElements;

}

void write_buffer(char *filename, char *buffer) {
	
	FILE *fin;
	int i = 0, carac;
	if (fin = fopen(filename, "r")) {
		do {
			carac = fgetc(fin);
			if(feof(fin))
				break;
			buffer[i] = carac;
			i++;
		}while(1);
		fclose(fin);
	}
	else {
        	fprintf(stderr, "Error in opening %s file for reading... exiting\n", filename);
        	exit(1);
    	}	
}

int main(int argc, char** argv) {
#ifdef TIMING
    	setup_start = timing_get_time();
#endif
    	int iterations;
    	char *inputFile, *goldFile;
    	char *out_buffer, *gold_buffer;

    	if (argc == 4) {
        	inputFile = argv[1];
        	goldFile = argv[2];
        	iterations = atoi(argv[3]);
    	} else {
        	fprintf(stderr, "Usage: %s <input file> <gold file> <#iterations>\n", argv[0]);
        	exit(1);
    	}

	char input[50], gold[50], *trash;
    	char *objects, *features, *clusters, *linkage;

	strcpy(input, inputFile);
	strcpy(gold, goldFile);

	trash = strtok(input, "/");
	if(trash != NULL) {
		while(1) {
			strcpy(input, trash);
			trash = strtok(NULL, "/");
			if(trash == NULL)
				break;
		}
		objects = strtok(input, "_");
		features = strtok(NULL, ".");
	}
	else {
		objects = strtok(NULL, "_");
		features = strtok(NULL, ".");
	}

	trash = strtok(gold, "/");
	if(trash != NULL) {
		while(1) {
			strcpy(gold, trash);
			trash = strtok(NULL, "/");
			if(trash == NULL)
				break;
		}
		clusters = strtok(gold, "_");
		linkage = strtok(NULL, "_");
	}
	else {
		clusters = strtok(NULL, "_");
		linkage = strtok(NULL, "_");
	}	
	
#ifdef LOGS
    	set_iter_interval_print(5);

	char test_info[200]; 
	snprintf(test_info, 200, "objects:%s features:%s clusters:%s linkage:%s",objects,features,clusters,linkage);
    	start_log_file("Agnes", test_info);
#endif
    	int size = readTxtFile(goldFile);
	if (size != ERROR) {
		gold_buffer = (char *)malloc(size*sizeof(char));
		write_buffer(goldFile, gold_buffer);
	}
	else {
		fprintf(stderr, "ERROR on readTxtFile for gold\n");
        	exit(1);
	}

#ifdef TIMING
    	setup_end = timing_get_time();
#endif
    	int loop;
    	for(loop=0; loop<iterations; loop++) {
#ifdef TIMING
        	loop_start = timing_get_time();
#endif
#ifdef ERR_INJ
        	if(loop == 2) {
            		printf("injecting errors, changing input!\n");

			/* Change the initials '1' in the file for '2' */
            		changeInput(inputFile);

        	} else if (loop == 3) {
            		printf("get ready, infinite loop...\n");
            		fflush(stdout);
            		while(1) {
                		sleep(100);
            		}
        	}
#endif

#ifdef TIMING
        	kernel_start = timing_get_time();
#endif
#ifdef LOGS
        	start_iteration();
#endif
		init_agnes(inputFile, clusters, linkage);
#ifdef LOGS
        	end_iteration();
#endif
#ifdef TIMING
        	kernel_end = timing_get_time();
#endif
		size = readTxtFile(outputFile);
		if (size != ERROR) {
			out_buffer = (char *)malloc(size*sizeof(char));
			write_buffer(outputFile, out_buffer);
		}
		else {
			fprintf(stderr, "ERROR on readTxtFile for out\n");
        		exit(1);
		}
#ifdef TIMING
        	check_start = timing_get_time();
#endif
        	int errors=0;
        	int i;
        
        	for(i=0; i< size; i++) {
            		if(out_buffer[i] != gold_buffer[i]) {
                		errors++;
                		char error_detail[200];
                		sprintf(error_detail," p: [%d], r: %c, e: %c", i, out_buffer[i], gold_buffer[i]);
#ifdef LOGS
				if (errors<500) //only writes in log the first 500 errors
                			log_error_detail(error_detail);
#endif
            		}
        	}
#ifdef LOGS
        	log_error_count(errors);
#endif
#ifdef TIMING
        	check_end = timing_get_time();
#endif
        	if(errors > 0) {
            		printf("Errors: %d\n",errors);
            		write_buffer(goldFile, gold_buffer);
        	} else {
            		printf("No errors\n");
        	}
        	
#ifdef TIMING
        	loop_end = timing_get_time();
        	double setup_timing = (double) (setup_end - setup_start) / 1000000;
        	double loop_timing = (double) (loop_end - loop_start) / 1000000;
        	double kernel_timing = (double) (kernel_end - kernel_start) / 1000000;
        	double check_timing = (double) (check_end - check_start) / 1000000;
        	printf("\n\tTIMING:\n");
        	printf("setup: %f\n",setup_timing);
        	printf("loop: %f\n",loop_timing);
        	printf("kernel: %f\n",kernel_timing);
        	printf("check: %f\n",check_timing);
#endif

    	}
#ifdef LOGS
    	end_log_file();
#endif
	free(out_buffer);
	free(gold_buffer);
	return 0;
}	
