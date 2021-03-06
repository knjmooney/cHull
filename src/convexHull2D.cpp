/******************************************************
 * Name    : convexHull2D.cpp
 * Author  : Kevin Mooney
 * Created : 25/07/16
 * Updated : 03/08/16
 *
 * Description:
 *
 * NOTES:
 *  - Graham Scan last loop doesn't work as expected
 *    for large data sets, most likley a problem with
 *    the logic
 ******************************************************/

#include <deque>
#include <list>
#include <vector>

#include "geometry.hpp"
#include "point.hpp"
#include "pointOperations.hpp"

using namespace std;

// Gift wrap algorithm
vector< size_t > giftWrap(const CompGeom::Geometry &geom) {
  if ( geom.getDim() != 2 ) {    
    errorM("Can only gift wrap 2D geometries\n");
  }  
  if ( geom.size() < 2 ) {
    errorM("Need more than 2 points to gift wrap\n");
  }
 
  // Fill a vector of indices
  vector<size_t> idx(geom.size());
  iota ( idx.begin(), idx.end(), 0 );

  // Find index of point furthest to left
  size_t nextID = -1;
  size_t curID  = *min_element ( idx.begin(), idx.end(), 
				 [&geom] ( size_t i, size_t j ) 
				 {
				   return geom[i][0] < geom[j][0];
				 } );

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
  } while ( cHull.front() != curID );
  cHull.push_back ( curID ) ;

  return std::vector<size_t>(cHull.begin(),cHull.end());
}

// Graham Scan algorithm
// The last loop is buggy
vector< size_t > grahamScan(const CompGeom::Geometry &geom_orig) {
  CompGeom::Geometry geom = geom_orig;

  if ( geom.getDim() != 2 ) {    
    errorM("Can only graham scan 2D geometries\n");
  }  
  if ( geom.size() < 3 ) {
    errorM("Need more than 2 points to do Graham Scan\n");
  }

  // Find average coordinate, set it to be the new origin
  CompGeom::Point aveTimesN = std::accumulate ( geom.begin(), geom.end(), CompGeom::Point({0,0}) );
  geom.translate( { - aveTimesN[0] / float(geom.size()), - aveTimesN[1] / float(geom.size()) } );

  // Initialise index arrays
  vector<size_t> idx(geom.size());
  iota ( idx.begin(), idx.end(), 0 );

  // Fill angles with the angle each line op
  // makes with the x-axis, from [-pi,pi]
  vector<float> angles(geom.size());
  transform ( geom.begin(), geom.end(), angles.begin(), [] ( CompGeom::Point p ) 
	      {
		return atan2 ( p[1], p[0] );
	      } );

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
  // This doesn't work as expected!!!
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
    // cout << u << " " << v << " " << rotation << endl;
    if ( rotation > 0 ) {
      ;;
    }
    else {
      imperfect_convex_hull = true;
      cHull_index.pop_back();
      N--;
    }

    u = geom[h1] - geom[h0];
    v = geom[h0] - geom[cHull_index[N-1]]; // Possible solution to bug
    rotation = cross2Product ( v,u );
    // cout << u << " " << v << " " << rotation << endl;
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

