/******************************************************
 * Name    : main.cpp
 * Author  : Kevin Mooney
 * Created : 13/06/16
 * Updated : 30/06/16
 *
 * Description:
 *
 *
 * NOTES:
 *  Graham Scan has a bug when executing the final loop
 ******************************************************/

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <limits>
#include <list>

#include "convexHull.hpp"
#include "geometry.hpp"
#include "errorMessages.hpp"
#include "pointOperations.hpp"
#include "cudaHull.hpp"
#include "triangle.hpp"
#include "edge.hpp"

using namespace std;
using namespace std::chrono;

const string filename = "data/cHull4Vertices.dat";

// Macro for timing function calls
#define timer(fun) {							\
    steady_clock::time_point t1 = steady_clock::now();			\
    (fun);								\
    steady_clock::time_point t2 = steady_clock::now();			\
    duration<float> time_span = duration_cast<duration<float>>(t2 - t1); \
    cout << time_span.count() << endl;					\
  }

// Gift wrap algorithm
vector< size_t > giftWrap(const CompGeom::Geometry &geom) {
  if ( geom.getDim() != 2 ) {    
    errorM("Can only gift wrap 2D geometries\n");
  }  
  if ( geom.size() < 2 ) {
    errorM("Need more than 2 points to gift wrap\n");
  }
 
  // Fill a vector of indeces
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

// Graham Scan algorithm
// The last loop is buggy
vector< size_t > grahamScan(CompGeom::Geometry &geom) {
  if ( geom.getDim() != 2 ) {    
    errorM("Can only graham scan 2D geometries\n");
  }  
  if ( geom.size() < 3 ) {
    errorM("Need more than 2 points to do Graham Scan\n");
  }

  // Find average coordinate, set it to be the new origin
  float avex = 0, avey = 0;
  for ( auto p : geom ) {
    avex += p[0];
    avey += p[1];
  }
  // // Alternative method using stl
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

// Helper function
// Finds the dot product between the norm of a triangle and 
// a ray from some point to the com of the triangle
// If it is negative, the triangle face is visible to the point
bool isTrinagleVisible ( const CompGeom::Triangle &t, const CompGeom::Point &p) {
  const CompGeom::Point &n = t.normal();
  const CompGeom::Point  m = p - t.com();
  return n[0]*m[0] + n[1]*m[1] + n[2]*m[2] < 0;
}

// Helper function
// Removes an element if it's already in the list, otherwise adds it.
// list is O(n) search which is bad
void addPotentialEdge ( list<CompGeom::Edge> &arr, size_t id0, size_t id1 ) {
  CompGeom::Edge E {id0, id1};
  auto it = find (arr.begin(),arr.end(), E ); 
  if ( it == arr.end() )
    arr.insert ( it, E );
  else
    arr.erase  ( it );
}

// As each triangle is oriented with some normal, all edges are entered
// such that the vertices are ordered anti-clockwise when viewing triangle from 
// the normal
vector < vector < size_t > > insertion3D ( const CompGeom::Geometry &geom ) {
  if ( geom.size() < 4 ) errorM ( "3D hull must have at least 4 points" ); 
    
  // Construct initial triangle
  list < CompGeom::Triangle > T = { { 0, 1, 2, geom } };
  CompGeom::Triangle & t0 = T.front();

  // Next point can't be visible from initial triangle
  if ( t0.isVisible ( geom[3] ) ) t0.invert();

  // Debugging
  geom.print3DGeom ( filename, geom.size()-2);
  {				// Debugging
    vector < vector < size_t > > result;
    for ( auto tri : T ) {
      result.push_back ( {tri[0],tri[1],tri[2]} );
    }
    geom.append3DHull( filename, result, 0);
  }
  
  // Construct hull of first 4 points being careful to enter edges in correct order
  T.insert( T.end(), { t0[0], t0[2], 3, geom } );
  T.insert( T.end(), { t0[2], t0[1], 3, geom } );
  T.insert( T.end(), { t0[1], t0[0], 3, geom } );
  
  { // Debugging
    vector < vector < size_t > > result;
    for ( auto tri : T ) {
      result.push_back ( {tri[0],tri[1],tri[2]} );
    }
    geom.append3DHull( filename, result, 1);
  }
  
  
  for ( size_t i=4; i<geom.size(); i++ ) {
    list < CompGeom::Edge > potential_edges;
    for ( auto it = T.begin(); it!=T.end(); it++ ) {
      auto &tri = *it;
      if ( tri.isVisible( geom[i] ) ) { 
	addPotentialEdge ( potential_edges, tri[0], tri[1] );
	addPotentialEdge ( potential_edges, tri[1], tri[2] );
	addPotentialEdge ( potential_edges, tri[2], tri[0] );
	T.erase (it++); it--;
      }
    }
    for ( auto edge : potential_edges ) {
      T.insert ( T.end(), {i,edge.first, edge.second,geom} );
    }
    // Debugging
    {			      
      vector < vector < size_t > > result;
      for ( auto tri : T ) {
	result.push_back ( {tri[0],tri[1],tri[2]} );
      }
      geom.append3DHull( filename, result, i-2);
    }
  }
  
  vector < vector < size_t > > result;
  for ( auto tri : T ) {
    result.push_back ( {tri[0],tri[1],tri[2]} );
  }
  return result;
} 


int main() {
  // CompGeom::Geometry geom = {{0,0,0}, {1,0,0}, {0,1,0}, {0,0,1}, {1,1,1},{0.5,0.5,0.5},{2,2,2}};
  CompGeom::Geometry geom{3};

  geom.addRandom(20000);

  vector < vector < size_t > > result;
  timer ( insertion3D(geom) );
  
  

  // vector < size_t > hull1;// = cudaHull(geom);
  // vector < size_t > hull2;// = grahamScan(geom);

  // cout << "giftWrap ";   timer ( giftWrap(geom) );
  // cout << "grahamScan "; timer ( hull1 = grahamScan(geom) );
  // cout << "cudaHull "; timer ( hull2 = cudaHull ( geom ) );

  // geom.print(); cout << "\n\n";
  // for ( auto i : hull1 ) cout << geom[i] << endl;


  // Check the answers match
  // sort ( hull1.begin(), hull1.end() );
  // sort ( hull2.begin(), hull2.end() ); 
  // unique ( hull1.begin(), hull1.end() );
  // unique ( hull2.begin(), hull2.end() );  
  // if ( hull1 == hull2 ) cout << "Hulls match\n";
  // else cout << "Hulls DON'T match\n";

  // cout << "\n\n";
  // for ( size_t i=0; i<hull1.size(); i++ ) {
  //   cout << hull2[i] << " " << hull1[i] << endl;
  // }  

  return EXIT_SUCCESS;
}
