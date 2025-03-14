ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
LDFLAGS=-L/usr/local/lib -lpthread -latomic -Ltbb2020/lib/intel64/gcc4.8 -ltbb
BLISS_LDFLAGS=-L$(ROOT_DIR)/core/bliss-0.73/ -lbliss
CFLAGS=-O3 -std=c++2a -Wall -Wextra -Wpedantic -fPIC -fconcepts -I$(ROOT_DIR)/core/ -Itbb2020/include -lcaf_core -lcaf_io
OBJ=core/DataGraph.o core/PO.o core/utils.o core/PatternGenerator.o $(ROOT_DIR)/core/showg.o
OUTDIR=bin/
CC=g++

all: bliss fsm count test existence-query convert_data count_distributed

core/roaring.o: core/roaring/roaring.c
	gcc -c core/roaring/roaring.c -o $@ -O3 -Wall -Wextra -Wpedantic -fPIC 

%.o: %.cc
	$(CC) -c $? -o $@ $(CFLAGS)

fsm: apps/fsm.cc $(OBJ) core/roaring.o bliss
	$(CC) apps/fsm.cc $(OBJ) core/roaring.o -o $(OUTDIR)/$@ $(BLISS_LDFLAGS) $(LDFLAGS) $(CFLAGS)

fsm_distributed: apps/fsm_distributed.cc $(OBJ) core/roaring.o bliss
	$(CC) apps/fsm_distributed.cc $(OBJ) core/roaring.o -o $(OUTDIR)/$@ $(BLISS_LDFLAGS) $(LDFLAGS) $(CFLAGS)

existence-query: apps/existence-query.cc $(OBJ) bliss
	$(CC) apps/existence-query.cc $(OBJ) -o $(OUTDIR)/$@ $(BLISS_LDFLAGS) $(LDFLAGS) $(CFLAGS)

existence-query-dist: apps/existence-query-dist.cc $(OBJ) bliss
	$(CC) apps/existence-query-dist.cc $(OBJ) -o $(OUTDIR)/$@ $(BLISS_LDFLAGS) $(LDFLAGS) $(CFLAGS)

enumerate: apps/enumerate.cc $(OBJ) bliss
	$(CC) apps/enumerate.cc $(OBJ) -o $(OUTDIR)/$@ $(BLISS_LDFLAGS) $(LDFLAGS) $(CFLAGS)

enumerate_distributed: apps/enumerate_distributed.cc $(OBJ) bliss
	$(CC) apps/enumerate_distributed.cc $(OBJ) -o $(OUTDIR)/$@ $(BLISS_LDFLAGS) $(LDFLAGS) $(CFLAGS)

count: apps/count.cc $(OBJ) bliss
	$(CC) apps/count.cc $(OBJ) -o $(OUTDIR)/$@ $(BLISS_LDFLAGS) $(LDFLAGS) $(CFLAGS)

count_distributed: apps/count_distributed.cc $(OBJ) bliss
	$(CC) apps/count_distributed.cc $(OBJ) -o $(OUTDIR)/$@ $(BLISS_LDFLAGS) $(LDFLAGS) $(CFLAGS)

output: apps/output.cc $(OBJ) bliss
	$(CC) apps/output.cc $(OBJ) -o $(OUTDIR)/$@ $(BLISS_LDFLAGS) $(LDFLAGS) $(CFLAGS)

test: core/test.cc $(OBJ) core/DataConverter.o core/roaring.o bliss
	$(CC) core/test.cc -DTESTING $(OBJ) core/DataConverter.o core/roaring.o -o $(OUTDIR)/$@ $(BLISS_LDFLAGS) $(LDFLAGS) -lUnitTest++ $(CFLAGS)

convert_data: core/convert_data.cc core/DataConverter.o core/utils.o
	$(CC) -o $(OUTDIR)/$@ $? $(LDFLAGS) $(CFLAGS)

bliss:
	make -C ./core/bliss-0.73

clean:
	make -C ./core/bliss-0.73 clean
	rm -f core/*.o bin/*
