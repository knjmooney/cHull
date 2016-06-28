/******************************************************
 * Name    : main.cpp
 * Author  : Kevin Mooney
 * Created : 13/06/16
 * Updated : 22/06/16
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

using namespace std;
using namespace std::chrono;

#define timer(fun) {							\
    steady_clock::time_point t1 = steady_clock::now();			\
    (fun);								\
    steady_clock::time_point t2 = steady_clock::now();			\
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1); \
    cout << time_span.count() << endl;					\
  }

// Gift wrap algorithm, this is much messier than I expected
vector< CompGeom::Point > giftWrap(const CompGeom::Geometry &geom) {
  if ( geom.getDim() != 2 ) {    
    errorM("Can only gift wrap 2D geometries\n");
  }  
  if ( geom.size() < 2 ) {
    errorM("Need more than 2 points to gift wrap\n");
  }
 
  vector < CompGeom::Point > cHull;

  // Find leftmost point and push it onto the hull
  auto first_on_hull_it = findMinX ( geom );
  auto curr_on_hull_it  = first_on_hull_it;
  CompGeom::Point dir_of_gift_wrap ( { 0,1 } );
  CompGeom::Point point_on_hull ( *curr_on_hull_it );
  cHull.push_back ( point_on_hull );

  // Assume there is at least more than one point
  do { 
    auto max_it = curr_on_hull_it;
    double max_angle = -numeric_limits<double>::max();	
    CompGeom::Point dir_to_point = dir_of_gift_wrap;

    // Loop over all points and calculate the angle
    // made with the gift wrap
    for ( auto it = geom.begin(); it != geom.end(); it++ ) {
      if ( curr_on_hull_it != it ) {
	dir_to_point = *it-*curr_on_hull_it;
	double angle = cosv2 ( dir_of_gift_wrap, dir_to_point ) ;
	// cout << *it << "\t" << angle << endl;
	if ( angle > max_angle ) { 
	  max_angle = angle;
	  max_it = it;
	}
      }
    }
    // cout << *max_it << "\t" << *curr_on_hull_it << endl; 
    dir_of_gift_wrap = *max_it - *curr_on_hull_it;;
    curr_on_hull_it = max_it;
    cHull.push_back ( *curr_on_hull_it );
    
  // } while (0);
  } while ( first_on_hull_it != curr_on_hull_it );
  return cHull;
}

vector< CompGeom::Point > grahamScan(CompGeom::Geometry &geom) {
  if ( geom.getDim() != 2 ) {    
    errorM("Can only graham scan 2D geometries\n");
  }  
  if ( geom.size() < 3 ) {
    errorM("Need more than 2 points to do Graham Scan\n");
  }

  // Find average coordinate set it to be the new origin
  double avex = 0, avey = 0;
  for ( auto p : geom ) {
    avex += p[0];
    avey += p[1];
  }
  avex /= double(geom.size());
  avey /= double(geom.size());
  geom.translate({-avex,-avey});

  // Initialise index arrays
  vector<size_t> idx(geom.size());
  for (size_t i = 0; i != idx.size(); ++i) idx[i] = i;

  // Fill angles with the angle each line op
  // makes with the x-axis, from [-pi,pi]
  vector<double> angles(geom.size());
  for ( size_t i=0; i<angles.size(); i++ ) {
    angles[i] = atan2 ( geom[i][1], geom[i][0] );
  }

  // sort indexes based on comparing values in v
  // Shamelessly stolen from Stack Exchange
  sort(idx.begin(), idx.end(),
       [&angles](size_t i1, size_t i2) {return angles[i1] < angles[i2];});

  // for ( size_t i=0; i<angles.size(); i++ ) {
  //   cout << geom[idx[i]] << endl; 
  // }

  deque < size_t > cHull_index;
  int h1 = idx[0], h2=idx[1], h3=idx[2];
  for ( size_t i=2; i<geom.size(); i++ ) {
    h3 = idx[i];

    CompGeom::Point u ( geom[h2] - geom[h1] ), v ( geom[h3] - geom[h2] );
    double rotation = cross2Product( u, v );

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
  // cHull.push_back(cHull[0]);

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
    double rotation;
    
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
  vector < CompGeom::Point > cHull;
  for ( size_t index : cHull_index ) {
    cHull.push_back ( geom[index] );
  }
  
  return cHull;
}

int main() {
  CompGeom::Geometry geom{2};
  
  geom.addRandom(1000);

  // timer ( giftWrap(geom) );
  // timer ( grahamScan(geom) );

  // vector < CompGeom::Point > hull  = giftWrap(geom);
  vector < CompGeom::Point > hull2 = grahamScan(geom);
  
  geom.print();
  cout << "\n\n";
  for ( auto i : hull2 ) {
    cout << i << endl;
  }

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
