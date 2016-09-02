CC  = nvcc

BIN     = ../bin
OBJ     = $(BIN)/convexHull2D.o $(BIN)/cudaHull.o $(BIN)/insertion3D.o $(BIN)/pba2DHost.o $(BIN)/gHullSerial.o $(BIN)/boundingBox.o $(BIN)/geometryHelper.o $(BIN)/voronoi.o
INC     = -I. -I../lib/ -I../src/

all: t/wvtest

t/wvtest: wvtestmain.cc wvtest.cc t/wvtest.t.cc $(OBJ)
	$(CC) -std=c++11 -D WVTEST_CONFIGURED -o $@ $(INC) $^

runtests: all
	t/wvtest

test: all
	./wvtestrun $(MAKE) runtests

clean::
	rm -f *~ t/*~ *.o t/*.o t/wvtest
