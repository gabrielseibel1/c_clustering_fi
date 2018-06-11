CC = gcc
CC_FLAGS = -std=c99 
CC_BUILD = -lm 

GOLD_GEN=./gold_gen/gold_gen
GOLD_CHECK=./gold_gen/agglomerate.c
SOURCES_CHECK=agnes_check.c ../../include/log_helper.c

all: gold agnes_check agnes_err_inj agnes_timing

gold: $(GOLD_CHECK)
	$(CC) $(CC_FLAGS) -o $(GOLD_GEN) $(GOLD_CHECK) $(CC_BUILD)

agnes_check: $(SOURCES_CHECK)
	$(CC) $(CC_FLAGS) -o agnes_check $(SOURCES_CHECK) -DLOGS $(CC_BUILD)

agnes_err_inj: $(SOURCES_CHECK)
	$(CC) $(CC_FLAGS) -o agnes_err_inj $(SOURCES_CHECK) -DLOGS -DERR_INJ $(CC_BUILD) 

agnes_timing: $(SOURCES_CHECK)
	$(CC) $(CC_FLAGS) -o agnes_timing $(SOURCES_CHECK) -DTIMING $(CC_BUILD)

clean:
	rm -f gold_gen/gold_gen agnes_check agnes_err_inj agnes_timing gold_gen/*.txt input_gen/*.txt *.txt
