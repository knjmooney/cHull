
OBJ = ../bin/convexHull2D.o ../bin/convexHull3D.o

all: t/wvtest

t/wvtest: wvtestmain.cc wvtest.cc t/wvtest.t.cc $(OBJ)
	g++ -std=c++11 -D WVTEST_CONFIGURED -o $@ -I. $^

runtests: all
	t/wvtest

test: all
	./wvtestrun $(MAKE) runtests

clean::
	rm -f *~ t/*~ *.o t/*.o t/wvtest
