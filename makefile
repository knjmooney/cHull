MAKE   = make
CXX    = g++
NVCC   = nvcc
LINKER = nvcc

CFLAGS    = -pg -O2 #-fdiagnostics-color=auto #-finline-functions -ffast-math -funroll-loops 
CXXFLAGS  = -pedantic -W -Wall -Wextra --std=c++11 $(CFLAGS)
NVCCFLAGS = --use_fast_math --std=c++11 

SRC       = ./src
BIN       = ./bin
INCPATH   = -I$(SRC)

TARGET    = $(BIN)/main.o $(BIN)/cudaHull.o
EXEC      = $(BIN)/convexHull


all: $(EXEC)

$(EXEC): $(TARGET)
	$(LINKER) -pg -o ${EXEC} ${TARGET} 	

$(BIN)/%.o: $(SRC)/%.cu makefile
	$(NVCC) $(NVCCFLAGS) -c $(INCPATH) $< -o $@

$(BIN)/%.o: $(SRC)/%.cpp makefile
	$(CXX) $(CXXFLAGS) -c $(INCPATH) $< -o $@

plot: $(EXEC)
	$(EXEC) > temp.dat 
	gnuplot -e "plot 'temp.dat' u 1:2 index 0 t '', 'temp.dat' index 1 t '' w lp; pause mouse any"
	rm temp.dat

test: $(EXEC)
	$(EXEC)

unit-tests: $(EXEC)
	make -C unit_tests test

install:

clean:
	rm -f ${TARGET} ${EXEC}
