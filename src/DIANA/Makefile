# C compiler
CC = gcc
CC_FLAGS = -fopenmp -O3

gen_bin: diana_clustering.c cluster.c getopt.c diana.c diana_gold_check.c 
	$(CC)  $(CC_FLAGS) diana_clustering.c cluster.c getopt.c diana.c -o diana
	$(CC)  $(CC_FLAGS) diana_clustering.c cluster.c getopt.c diana_gold_check.c -DLOGS -o diana_gold_check
 
clean:
	rm -f *.o *~ diana diana_gold_check
