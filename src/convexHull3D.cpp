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
#include <string>
#include <vector>

#include "geometry.hpp"
#include "point.hpp"
#include "triangle.hpp"
#include "edge.hpp"
#include "boundingBox.hpp"
#include "pba2D.h"		// Parallel Banding Algorithm

#define EPS 1e-15		// Epsilon

// pba parameters, choice of parameters discussed in pba paper
// These are important, they determine the blocksizes
#define P1B 16			// Phase 1 band
#define P2B 16			// Phase 2 band
#define P3B 16			// Phase 3 band

#define BOXSIZE 512

using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  INSERTION 3D  /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  DEBUG VERSION  ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  GHULL SERIAL  /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

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

// Step 1
// Wrap the set of points in a box and project the points onto bricks on the faces
// deciding conflicts by choosing the closer point
void projectToTile ( CompGeom::BoundingBox &B, const CompGeom::Geometry &geom ) {
  CompGeom::Tile *Tmax[] = { B.right(), B.front(), B.up()   };
  CompGeom::Tile *Tmin[] = { B.left() , B.back() , B.down() };

  float min[3],max[3];
  findExtremes ( geom, min, max );

  // DEBUG
  // printf("max: %f %f %f\n",max[0],max[1],max[2] );
  // printf("min: %f %f %f\n",min[0],min[1],min[2] );

  // All tiles should have same width and height
  // i.e. box should be a cube
  int w = Tmin[0]->width();
  int h = Tmin[0]->height();

  for ( size_t i=0; i<3; i++ ) {
    // If i is the x-axis, j and k are y and z-axis accordingly
    // and then cyclic permutations of these
    int j = (i+1)%3;
    int k = (i+2)%3;
    for ( auto p : geom ) {
      // Find out the indexes of the brick on the tile the 
      // points are projected onto
      // Add small correction to denominator to keep values
      // inside [0,w)
      int idj = int(w*(p[j]-min[j])/((max[j]-min[j])*(1+EPS)));
      int idk = int(h*(p[k]-min[k])/((max[k]-min[k])*(1+EPS)));
	
      // Check is the current value on the brick
      // is less than value set
      float cmin_val = Tmin[i]->get(idj,idk);
      float cmax_val = Tmax[i]->get(idj,idk);
      float this_min = fabs(p[i]-min[i]);
      float this_max = fabs(p[i]-max[i]);
      // DEBUG
      // printf("idj:%d idk:%d cmin:%3e cmax:%3e this_min:%f this_max:%f p(%f,%f,%f)\n",
      // 	     idj, idk, cmin_val, cmax_val, this_min, this_max, p[0],p[1],p[2] );
      if ( cmin_val > this_min ) {
	Tmin[i]->set (idj,idk, this_min );
      }
      if ( cmax_val > this_max ) {
	Tmax[i]->set (idj,idk, this_max );
      }
      // DEBUG
      // cmin_val = Tmin[i]->get(idj,idk);
      // cmax_val = Tmax[i]->get(idj,idk);
      // printf("idj:%d idk:%d cmin:%3e cmax:%3e this_min:%f this_max:%f p(%f,%f,%f)\n\n",
      // 	     idj, idk, cmin_val, cmax_val, this_min, this_max, p[0],p[1],p[2] );
    }
  }
  // for ( int i=0; i<3; i++ ) {
  //   Tmax[i]->print();
  //   printf ("\n\n");
  // }
  // Tmax[0]->print();
  //   printf ("\n\n");
  // Tmin[0]->print();

}

size_t shortID ( int i, int j , int w) {
  return 2*(i*w + j);
}

void constructVoronois ( CompGeom::BoundingBox & B, const CompGeom::Geometry & geom) {
  size_t w = B.width();
  size_t h = B.height();
  
  // Initialise the input and output arrays 
  // with the marker value defined in the pba
  // header file
  short * input  = new short[2*w*h];
  short * output[6];
  for ( size_t i=0; i<6; i++ ) {
    output[i] = new short[2*w*h];
    // std::fill ( output[i], output[i] + 2*w*h, MARKER );
  }

  CompGeom::Tile *Tmax[] = { B.right(), B.front(), B.up()   };
  CompGeom::Tile *Tmin[] = { B.left() , B.back() , B.down() };

  // This method has been called previously 
  // when projecting onto the planes
  // O(N)
  float min[3],max[3];
  findExtremes ( geom, min, max );

  // Allocates memory for the pba library
  // I'm unsure what the parameter is that we are passing
  // But in the example main, this is what is passed for
  // an input array of size 2*w*w as we have here.
  pba2DInitialization(w);

  for ( int i=0; i<3; i++ ) {
    int j = (i+1)%3;
    int k = (i+2)%3;

    // Calculate Voronoi diagram on bottom face
    std::fill ( input, input + 2*w*h, MARKER );
    for ( auto p : geom ) {
      int idj = int(w*(p[j]-min[j])/((max[j]-min[j])*(1+EPS)));
      int idk = int(h*(p[k]-min[k])/((max[k]-min[k])*(1+EPS)));
      int index = shortID(idj,idk,w);

      if ( Tmin[i]->get(idj,idk) == fabs(p[i]-min[i]) ) {
	input[index]   = idk;
	input[index+1] = idj;
      }
    }
    pba2DVoronoiDiagram(input,output[2*i],P1B,P2B,P3B);

    std::fill ( input, input + 2*w*h, MARKER );
    for ( auto p : geom ) {
      int idj = int(w*(p[j]-min[j])/((max[j]-min[j])*(1+EPS)));
      int idk = int(h*(p[k]-min[k])/((max[k]-min[k])*(1+EPS)));
      int index = shortID(idj,idk,w);


      if ( Tmax[i]->get(idj,idk) < 100 ) 
	cout << Tmax[i]->get(idj,idk) << " " <<  fabs(p[i]-max[i]) 
	     << " " << index << " " << idj << " " << idk << endl;
      
      if ( Tmax[i]->get(idj,idk) == fabs(p[i]-max[i]) ) {
	input[index]   = idk;
	input[index+1] = idj;
      }
    }


    pba2DVoronoiDiagram(input,output[2*i+1],P1B,P2B,P3B);

    for ( size_t l=0; l<h; l++ ) {
      for ( size_t j=0; j<w; j++ ) {
	int index = shortID(l,j,w);
	if ( input[index] == MARKER ) cout << "- "; 
	else cout << "(" << input[index] << "," << input[index+1] << ") ";
      }
      cout << endl;
    }
    cout << endl << endl;;

    for ( size_t l=0; l<h; l++ ) {
      for ( size_t j=0; j<w; j++ ) {
	int index = shortID(l,j,w);
	if ( output[2*i+1][index] == MARKER ) cout << "- "; 
	else cout << "(" << output[2*i+1][index] << "," << output[2*i+1][index+1] << ") ";
      }
      cout << endl;
    }

    cout << endl << endl;
  }


  // Free all memory
  pba2DDeinitialization();
  delete[] input;
  for (int i=0; i<6; i++ ) delete[] output[i];
}

vector < vector < size_t > > gHullSerial ( const CompGeom::Geometry &geom ) {
  if ( geom.size() < 4 ) errorM("3D geometry needs at least for non-coplanar points to be a convex hull"); 
  if ( geom.getDim() != 3 ) errorM("gHullSerial only works in 3 dimensions");

  size_t w, h;
  w = h = BOXSIZE;

  CompGeom::BoundingBox B ( w,h );
  
  projectToTile(B,geom);
  constructVoronois ( B,geom );


  // for ( auto i : geom ) cout << i << endl;

  return vector < vector < size_t > > ( 1,{0,1,2} );
}

