
#include <chrono>
#include <iostream>
#include <string>


#include "convexHull2D.hpp"
#include "cudaHull.hpp"
#include "geometry.hpp"
#include "gHull.cuh"
#include "gHullSerial.hpp"
#include "insertion3D.hpp"

using namespace std;


#define timer(fun) {							\
    using namespace chrono;						\
    steady_clock::time_point t1 = steady_clock::now();			\
    (fun);								\
    steady_clock::time_point t2 = steady_clock::now();			\
    chrono::duration<float> time_span					\
      = chrono::duration_cast<chrono::duration<float>>(t2 - t1);	\
    printf("%15lf ", time_span.count());				\
  }


int main() {
  

  vector < int > sizes(10);
  generate(sizes.begin(),sizes.end(),[] () {
      static int range_min = 0, range_step = 1e6;
      return (range_min += range_step);
    });
   
  printf ( "%-8s %15s %15s %15s %15s %15s %15s\n",
	   "","Gift Wrap", "Graham Scan", "Monotone Chains",
	   "Insertion 3D", "gHull Serial", "gHull");

  for ( auto sz : sizes ) {
    printf ( "%8d ", sz );
    CompGeom::Geometry geom (2);
    geom.addRandom(sz);
    timer ( giftWrap (geom) );

    // for ( const auto & func : {giftWrap, grahamScan, cudaHull} ) { 
    //   CompGeom::Geometry geom (2);
    //   geom.addRandom(sz);
    //   timer ( func ( geom ) );
    // }

    // for ( const auto & func : {insertion3D, gHullSerial, gHull} ) { 
    //   CompGeom::Geometry geom (3);
    //   geom.addRandom(sz);
    //   timer ( func ( geom ) );
    // }
    printf("\n");
  }

  
}
