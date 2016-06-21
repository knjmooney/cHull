MAKE   = make
CXX    = g++
NVCC   = nvcc
LINKER = nvcc

CFLAGS    = -O3 -finline-functions -ffast-math -fomit-frame-pointer -funroll-loops \
	-fdiagnostics-color=auto
CXXFLAGS  = -g -pedantic -W -Wall -Wextra -L/usr/lib --std=c++11 $(CFLAGS)
NVCCFLAGS = --use_fast_math --std=c++11 

SRC       = ./src
BIN       = ./bin
INCPATH   = -I$(SRC)

TARGET    = $(BIN)/main.o
EXEC      = $(BIN)/convexHull


all: $(EXEC)

$(EXEC): $(TARGET)
	$(CXX) -Wall -o ${EXEC} ${TARGET}	
# $(LINKER) -o ${EXEC} ${TARGET} 	

%.o: %.cu makefile
	$(NVCC) $(NVCCFLAGS) -c $(INCPATH) $<

$(BIN)/%.o: $(SRC)/%.cpp makefile
	$(CXX) $(CXXFLAGS) -c $(INCPATH) $< -o $@

plot: $(EXEC)
	./bin/convexHull > temp.dat 
	gnuplot -e "plot 'temp.dat' index 0 t '', 'temp.dat' index 1 t '' w lp; pause mouse any"
	rm temp.dat

install:

clean:
	rm -f *.o ${TARGET} ${EXEC}
