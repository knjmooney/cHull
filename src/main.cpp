/******************************************************
 * Name    : main.cpp
 * Author  : Kevin Mooney
 * Created : 13/06/16
 * Updated : 30/06/16
 *
 * Description:
 *
 ******************************************************/

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <limits>

#include "convexHull.hpp"
#include "geometry.hpp"
#include "errorMessages.hpp"
#include "pointOperations.hpp"
#include "cudaHull.hpp"

using namespace std;
using namespace std::chrono;

#define timer(fun) {							\
    steady_clock::time_point t1 = steady_clock::now();			\
    (fun);								\
    steady_clock::time_point t2 = steady_clock::now();			\
    duration<float> time_span = duration_cast<duration<float>>(t2 - t1); \
    cout << time_span.count() << endl;					\
  }

// Gift wrap algorithm, this is much messier than I expected
vector< size_t > giftWrap(const CompGeom::Geometry &geom) {
  if ( geom.getDim() != 2 ) {    
    errorM("Can only gift wrap 2D geometries\n");
  }  
  if ( geom.size() < 2 ) {
    errorM("Need more than 2 points to gift wrap\n");
  }
 
  // Fill a vector of indexes
  vector<size_t> idx(geom.size());
  for (size_t i = 0; i != idx.size(); ++i) idx[i] = i;

  // Find index of point furthest to left
  size_t nextID = -1;
  size_t curID  = *min_element ( idx.begin(), idx.end(), 
				 [&geom] ( size_t i, size_t j ) 
				 { return geom[i][0] < geom[j][0]; } );

  CompGeom::Point dir_of_gift_wrap {{ 0,1 }};
  CompGeom::Point dir_to_point     {{ 0,0 }};

  // Loop until we arrive back to the start
  vector < size_t > cHull;
  do { 
    cHull.push_back ( curID ) ;
    float max_angle = -numeric_limits<float>::max();	
    nextID = -1;
    // Find the smallest angle... contradictory naming convention 
    for ( size_t i=0; i<geom.size(); i++ ) {
      if ( i != curID ) {
    	dir_to_point = geom[i] - geom[curID];
    	float angle = cosv2 ( dir_of_gift_wrap, dir_to_point );
    	if ( angle > max_angle ) {
    	  max_angle = angle;
    	  nextID    = i;
    	}
      }
    }
    dir_of_gift_wrap = geom[nextID] - geom[curID];
    curID = nextID;
  } while ( cHull[0] != curID );
  cHull.push_back ( curID ) ;

  return cHull;
}

vector< size_t > grahamScan(CompGeom::Geometry &geom) {
  if ( geom.getDim() != 2 ) {    
    errorM("Can only graham scan 2D geometries\n");
  }  
  if ( geom.size() < 3 ) {
    errorM("Need more than 2 points to do Graham Scan\n");
  }

  // Find average coordinate set it to be the new origin
  float avex = 0, avey = 0;
  for ( auto p : geom ) {
    avex += p[0];
    avey += p[1];
  }
  // std::accumulate ( geom.begin(), geom.end(), avex, 
  // 		    [] (int x, CompGeom::Point y) { return x + y[0]; } );
  // std::accumulate ( geom.begin(), geom.end(), avey, 
  // 		    [] (int x, CompGeom::Point y) { return x + y[1]; } );
  avex /= float(geom.size());
  avey /= float(geom.size());
  geom.translate({-avex,-avey});


  // Initialise index arrays
  vector<size_t> idx(geom.size());
  for (size_t i = 0; i != idx.size(); ++i) idx[i] = i;

  // Fill angles with the angle each line op
  // makes with the x-axis, from [-pi,pi]
  vector<float> angles(geom.size());
  for ( size_t i=0; i<angles.size(); i++ ) {
    angles[i] = atan2 ( geom[i][1], geom[i][0] );
  }

  // sort indexes based on comparing values in v
  // Shamelessly stolen from Stack Exchange
  sort(idx.begin(), idx.end(),
       [&angles](size_t i1, size_t i2) {return angles[i1] < angles[i2];});

  // Start Graham Scan
  deque < size_t > cHull_index;
  int h1 = idx[0], h2=idx[1], h3=idx[2];
  for ( size_t i=2; i<geom.size(); i++ ) {
    h3 = idx[i];

    CompGeom::Point u ( geom[h2] - geom[h1] ), v ( geom[h3] - geom[h2] );
    float rotation = cross2Product( u, v );

    while ( rotation < 0 && cHull_index.size() >= 2) {
      h2 = h1;      
      h1 = cHull_index.back();
      cHull_index.pop_back();
      u = geom[h2] - geom[h1];
      v = geom[h3] - geom[h2];
      rotation = cross2Product ( u,v );
    }
    
    if ( rotation > 0 ) {
      cHull_index.push_back(h1);
      h1 = h2;
      h2 = h3;
    }
    else {
      h2 = h3;
    }
  }
  cHull_index.push_back(h1);
  cHull_index.push_back(h2);

  // Fix the start and end
  bool imperfect_convex_hull = true;
  int N = cHull_index.size();
  int hnm2,hnm1, h0;
  while ( imperfect_convex_hull && N > 3 ) {

    imperfect_convex_hull = false;
    hnm2 = cHull_index[N-2];
    hnm1 = cHull_index[N-1];
    h0   = cHull_index[0];
    h1   = cHull_index[1];
    
    CompGeom::Point u ( geom[hnm1] - geom[hnm2] ), v ( geom[h0] - geom[hnm1]);
    float rotation;
    
    rotation = cross2Product ( u,v );
    if ( rotation > 0 ) {
      ;;
    }
    else {
      imperfect_convex_hull = true;
      cHull_index.pop_back();
      N--;
    }

   
    u = geom[h1] - geom[h0];
    rotation = cross2Product ( v,u );
    if ( rotation > 0 ) {
      ;;
    }
    else {
      imperfect_convex_hull = true;
      cHull_index.pop_front();
      N--;
    }
  }

  // Complete the loop
  cHull_index.push_back(cHull_index.front());
  
  // geom.translate({avex,avey});
  // vector < CompGeom::Point > cHull;
  // for ( size_t index : cHull_index ) {
  //   cHull.push_back ( geom[index] );
  // }
  
  return vector<size_t>(cHull_index.begin(),cHull_index.end());
}

int main() {
  CompGeom::Geometry geom{2};
  
  geom.addRandom(100);

  // timer ( giftWrap(geom) );
  // timer ( grahamScan(geom) );

  
  // vector < size_t > hull2 = grahamScan(geom);

  auto hull2 = giftWrap(geom);
  geom.print();
  cout << "\n\n";
  for ( auto i : hull2 ) {
    cout << geom[i] << endl;
  }  


  geom.printHull ( "data/test.dat", hull2 );

  cudaHull ( hull2 );

  // cout << "\n\n";
  // for ( auto i : hull ) {
  //   cout << i << endl;
  // }

  
  // CompGeom::Point d({1,0}),v({1, -1});

  // cout << cross2Product ( d,v ) << endl;
  
  // cout << innerProduct(v,d) << endl;
  // cout << norm(d) << endl;

  // cout << cosv2(v,d) << endl;

  // ClockwiseAngle(p1,p2);
  // cout << -p2 << endl;
  // cout << p2 << endl;

  return EXIT_SUCCESS;
}
