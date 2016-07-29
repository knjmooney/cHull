/******************************************************
 * Name    : convexHull3D.cpp
 * Author  : Kevin Mooney
 * Created : 25/07/16
 * Updated : 27/07/16
 *
 * Description:
 *
 * NOTES:
 ******************************************************/

#include <algorithm>
#include <list>
#include <string>
#include <vector>

#include "geometry.hpp"
#include "point.hpp"
#include "triangle.hpp"
#include "edge.hpp"
#include "boundingBox.hpp"

using namespace std;

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
  if ( geom.getDim() != 3 ) errorM ( "Insertion3D only works in 3 dimensions" );

  // Construct initial triangle
  list < CompGeom::Triangle > T = { { 0, 1, 2, geom } };
  CompGeom::Triangle & t0 = T.front();

  // Next point can't be visible from initial triangle
  // Potential problem if geom[3] is coplanar
  if ( t0.isVisible ( geom[3] ) ) t0.invert();

  // Construct hull of first 4 points being careful to enter edges in correct order
  T.insert( T.end(), { t0[0], t0[2], 3, geom } );
  T.insert( T.end(), { t0[2], t0[1], 3, geom } );
  T.insert( T.end(), { t0[1], t0[0], 3, geom } );
  
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
  }
  
  vector < vector < size_t > > result;
  for ( auto tri : T ) {
    result.push_back ( {tri[0],tri[1],tri[2]} );
  }
  return result;
} 


// As each triangle is oriented with some normal, all edges are entered
// such that the vertices are ordered anti-clockwise when viewing triangle from 
// the normal
vector < vector < size_t > > insertion3D ( const CompGeom::Geometry &geom, const std::string &filename ) {
  if ( geom.size() < 4 ) errorM ( "3D hull must have at least 4 points" ); 
    
  // Construct initial triangle
  list < CompGeom::Triangle > T = { { 0, 1, 2, geom } };
  CompGeom::Triangle & t0 = T.front();

  // Next point can't be visible from initial triangle
  // Potential problem if geom[3] is coplanar
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

////////////////////////////////// GHULL SERIAL //////////////////////////////////

// Helper Function
// Finds the minimum and maximum coordinates in all dimensions
void findExtremes ( const CompGeom::Geometry &geom, float min[3], float max[3] ) {
  // Set initial values for the mins and maxes
  for ( int i=0; i<3; i++ ) {
    min[i] = geom[0][i];
    max[i] = geom[0][i];
  }
  // Loop over all points and update min or max if necessary
  for ( auto p : geom ) {
    for ( int i=0; i<3; i++ ) {
      min[i] = p[i] > min[i] ? min[i] : p[i];
      max[i] = p[i] < max[i] ? max[i] : p[i];
    }
  }  
}

void projectToTile ( CompGeom::BoundingBox &B, const CompGeom::Geometry &geom ) {
  CompGeom::Tile *Tmax[] = { B.right(), B.front(), B.up()   };
  CompGeom::Tile *Tmin[] = { B.left() , B.back() , B.down() };

  float min[3],max[3];
  findExtremes ( geom, min, max );

  // All tiles should have same width and height
  // i.e. box should be a cube
  int w = Tmin[0]->width();
  int h = Tmin[0]->height();

  for ( size_t i=0; i<3; i++ ) {
    // If i is the x-axis, j and k and y and z-axis accordingly
    // and then cyclic permutations of these
    int j = (i+1)%3;
    int k = (i+2)%3;
    for ( auto p : geom ) {
      // Find out the indexs of the brick of the tile the 
      // points are projected onto
      int idj = int(w*(p[j]-min[j])/(max[j]-min[j]));
      int idk = int(h*(p[k]-min[k])/(max[k]-min[k]));
	
      // The current value on the brick
      float cmin_val = Tmin[i]->get(idj,idk);
      float cmax_val = Tmax[i]->get(idj,idk);
      if ( cmin_val > p[i] ) {
	Tmin[i]->set (idj,idk, p[i] );
      }
      if ( cmax_val < p[i] ) {
	Tmax[i]->set (idj,idk, p[i] );
      }
    }
  }
}

vector < vector < size_t > > gHullSerial ( const CompGeom::Geometry &geom ) {
  if ( geom.size() < 4 ) errorM("3D geometry needs at least for non-coplanar points to be a convex hull"); 
  if ( geom.getDim() != 3 ) errorM("gHullSerial only works in 3 dimensions");

  int w = 32, h = 32;

  CompGeom::BoundingBox B ( w,h );
  
  projectToTile(B,geom);
  
  for ( auto i : geom ) cout << i << endl;

  return vector < vector < size_t > > ( 1 );
}




