/******************************************************
 * Name    : convexHull3D.cpp
 * Author  : Kevin Mooney
 * Created : 25/07/16
 * Updated : 02/08/16
 *
 * Description:
 *
 * NOTES:
 ******************************************************/

#include <algorithm>
#include <list>
#include <limits>
#include <string>
#include <vector>

#include "geometry.hpp"
#include "point.hpp"
#include "triangle.hpp"
#include "unorderedEdge.hpp"

using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  INSERTION 3D  /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

// Deprecated
// This has been superseeded by the member function, Triangle::isVisible()
// Helper function
// Finds the dot product between the norm of a triangle and 
// a ray from some point to the com of the triangle
// If it is negative, the triangle face is visible to the point
inline bool isTrinagleVisible ( const CompGeom::Triangle &t, const CompGeom::Point &p) {
  const CompGeom::Point &n = t.normal();
  const CompGeom::Point  m = p - t.com();
  return n[0]*m[0] + n[1]*m[1] + n[2]*m[2] < 0;
}


// Helper function
// Removes an element if it's already in the list, otherwise adds it.
// list is O(n) search which is bad
void addPotentialEdge ( list<CompGeom::UnorderedEdge> &arr, size_t id0, size_t id1 ) {
  const CompGeom::UnorderedEdge	E  = {id0, id1};
  const auto			it = find (arr.begin(),arr.end(), E ); 
  if ( it == arr.end() )
    arr.insert ( it, E );
  else
    arr.erase  ( it );
}

// As each triangle is oriented with some normal, all edges are entered
// such that the vertices are ordered anti-clockwise when viewing triangle from 
// the normal
vector < vector < size_t > > insertion3D ( const CompGeom::Geometry &geom ) {
  if ( geom.size()    < 4 ) errorM ( "3D hull must have at least 4 points" ); 
  if ( geom.getDim() != 3 ) errorM ( "Insertion3D only works in 3 dimensions" );

  // Construct initial triangle
  list < CompGeom::Triangle > T = { { 0, 1, 2, geom } };
  CompGeom::Triangle &	t0	= T.front();

  // Next point can't be visible from initial triangle
  // Potential problem if geom[3] is coplanar
  if ( t0.isVisible ( geom[3] ) ) t0.invert();

  // Construct hull of first 4 points being careful to enter edges in correct order
  T.insert( T.end(), { t0[0], t0[2], 3, geom } );
  T.insert( T.end(), { t0[2], t0[1], 3, geom } );
  T.insert( T.end(), { t0[1], t0[0], 3, geom } );
  
  for ( size_t i=4; i<geom.size(); i++ ) {
    list < CompGeom::UnorderedEdge > potential_edges;
    for ( auto it = T.begin(); it != T.end(); it++ ) {
      auto	&tri		   = *it;
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

//////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  DEBUG VERSION  ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

// As each triangle is oriented with some	normal, all edges are entered
// such that the vertices are ordered anti-clockwise when viewing triangle from 
// the normal
vector < vector < size_t > > insertion3D ( const CompGeom::Geometry &geom, const std::string &filename ) {
  if ( geom.size() < 4 ) errorM ( "3D hull must have at least 4 points" ); 
    
  // Construct initial triangle
  list < CompGeom::Triangle > T = { { 0, 1, 2, geom } };
  CompGeom::Triangle &	t0	= T.front();

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
  
  {				// Debugging
    vector < vector < size_t > > result;
    for ( auto tri : T ) {
      result.push_back ( {tri[0],tri[1],tri[2]} );
    }
    geom.append3DHull( filename, result, 1);
  }
  
  
  for ( size_t i=4; i<geom.size(); i++ ) {
    list < CompGeom::UnorderedEdge > potential_edges;
    for ( auto it = T.begin(); it != T.end(); it++ ) {
      auto	&tri		   = *it;
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
