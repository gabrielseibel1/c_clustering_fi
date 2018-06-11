CC = gcc
CC_FLAGS = -std=c99 

SOURCES_CHECK=agnes_check.c ../../include/log_helper.c

all: gold agnes_check agnes_err_inj agnes_timing

gold: ./golden_files/agglomerate.c
	$(CC) $(CC_FLAGS) -o ./golden_files/gold_gen ./golden_files/agglomerate.c -lm -Wall

agnes_check: $(SOURCES_CHECK)
	$(CC) $(CC_FLAGS) -o agnes_check $(SOURCES_CHECK) -DLOGS -lm -Wall

agnes_err_inj: $(SOURCES_CHECK)
	$(CC) $(CC_FLAGS) -o agnes_err_inj $(SOURCES_CHECK) -DLOGS -DERR_INJ -lm -Wall 

agnes_timing: $(SOURCES_CHECK)
	$(CC) $(CC_FLAGS) -o agnes_timing $(SOURCES_CHECK) -DTIMING -lm -Wall

clean:
	rm -f golden_files/gold_gen agnes_check agnes_err_inj agnes_timing golden_files/*.txt input_gen/*.txt *.txt
