MAKE   = make
CXX    = g++
NVCC   = nvcc
LINKER = nvcc

PROF      = 
CFLAGS    = -g $(PROF) --std=c++11 #-O2 -finline-functions -ffast-math -funroll-loops 
CXXFLAGS  = -pedantic -W -Wall -Wextra --std=c++11 $(CFLAGS)
NVCCFLAGS = -G --use_fast_math $(CFLAGS)

LIB       = ./lib
SRC       = ./src
BIN       = ./bin
INCPATH   = -I$(SRC) -I$(LIB)

TARGET    = $(BIN)/main.o $(BIN)/convexHull2D.o $(BIN)/pba2DHost.o $(BIN)/voronoi.o $(BIN)/boundingBox.o $(BIN)/gHullSerial.o $(BIN)/geometryHelper.o $(BIN)/insertion3D.o #$(BIN)/cudaHull.o
EXEC      = $(BIN)/convexHull

TEST_ARGS = -a gHullSerial -d 3 -n 1000

all: $(EXEC)

$(EXEC): $(TARGET)
	$(LINKER) $(PROF) -o ${EXEC} ${TARGET} 	

$(BIN)/%.o: $(SRC)/%.cu makefile
	$(NVCC) $(NVCCFLAGS) -c $(INCPATH) $< -o $@

$(BIN)/%.o: $(LIB)/%.cu makefile
	$(NVCC) $(NVCCFLAGS) -c $(INCPATH) $< -o $@

$(BIN)/%.o: $(SRC)/%.cpp makefile
	$(CXX) $(CXXFLAGS) -c $(INCPATH) $< -o $@

debug: $(EXEC)
	cuda-gdb --args $(EXEC) $(TEST_ARGS) 

plot: $(EXEC)
	$(EXEC) > temp.dat 
	gnuplot -e "plot 'temp.dat' u 1:2 index 0 t '', 'temp.dat' index 1 t '' w lp; pause mouse any"
	rm temp.dat

test: $(EXEC)
	$(EXEC) $(TEST_ARGS)

unit-tests: $(EXEC)
	make -C unit_tests test

install:

clean:
	rm -f ${TARGET} ${EXEC}
