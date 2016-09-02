MAKE   = make
CXX    = g++
NVCC   = nvcc
LINKER = nvcc

PROF      = 
CFLAGS    =  $(PROF) --std=c++11 -O2 
CXXFLAGS  = -pedantic -W -Wall -Wextra $(CFLAGS)
NVCCFLAGS = -lineinfo --use_fast_math -arch=sm_35 -dc $(CFLAGS)
LDFLAGS   = -lcudadevrt

LIB       = ./lib
SRC       = ./src
BIN       = ./bin
INCPATH   = -I$(SRC) -I$(LIB)

TARGET    = $(BIN)/main.o $(BIN)/convexHull2D.o $(BIN)/pba2DHost.o $(BIN)/voronoi.o $(BIN)/boundingBox.o $(BIN)/gHullSerial.o $(BIN)/geometryHelper.o $(BIN)/insertion3D.o $(BIN)/gHull.o $(BIN)/cudaHull.o
EXEC      = $(BIN)/convexHull

TEST_ARGS = -a gHull -d 3 -n 10000

all: $(EXEC)

$(EXEC): $(TARGET)
	$(LINKER) $(PROF) -rdc=true -arch=sm_35 -o ${EXEC} ${TARGET} $(LDFLAGS)	

$(BIN)/%.o: $(SRC)/%.cu makefile
	$(NVCC) $(NVCCFLAGS) -c $(INCPATH) $< -o $@

$(BIN)/%.o: $(LIB)/%.cu makefile
	$(NVCC) $(NVCCFLAGS) -c $(INCPATH) $< -o $@

$(BIN)/%.o: $(SRC)/%.cpp makefile
	$(CXX) $(CXXFLAGS) -c $(INCPATH) $< -o $@


$(BIN)/benchmark: benchmark/benchmark.cpp
	$(MAKE) -C benchmark

debug: $(EXEC)
	cuda-gdb --args $(EXEC) $(TEST_ARGS) 

plot: $(EXEC)
	$(EXEC) > temp.dat 
	gnuplot -e "plot 'temp.dat' u 1:2 index 0 t '', 'temp.dat' index 1 t '' w lp; pause mouse any"
	rm temp.dat

test: $(EXEC)
	time $(EXEC) $(TEST_ARGS)

memcheck: $(EXEC)
	cuda-memcheck $(EXEC) $(TEST_ARGS)

unit-tests: $(EXEC)
	make -C unit_tests test

install:

clean:
	rm -f ${TARGET} ${EXEC}
