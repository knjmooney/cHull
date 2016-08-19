/******************************************************
 * Name    : gHullSerial.cpp
 * Author  : Kevin Mooney
 * Created : 16/08/16
 * Updated : 
 *
 * Description:
 *
 * NOTES:
 ******************************************************/

#include <algorithm>
#include <limits>
#include <string>
#include <vector>

#include "boundingBox.hpp"
#include "geometry.hpp"
#include "geometryHelper.hpp"
#include "orderedEdge.hpp"
#include "pba2D.h"		// Parallel Banding Algorithm
#include "voronoi.hpp"
#include "workingSet.hpp"

#define EPS	1e-5		// Epsilon

// pba parameters, choice of parameters discussed in pba paper
// These are important, they determine the blocksizes
#define P1B	16		// Phase 1 band
#define P2B	16		// Phase 2 band
#define P3B	16		// Phase 3 band

#define BOXSIZE 512
#define DIM     3

using namespace std;
using CompGeom::Geometry;
using CompGeom::BoundingBox;
using CompGeom::Tile;
using CompGeom::OrderedEdge;
using CompGeom::WorkingSet;

//////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  GHULL SERIAL  /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

// Maybe a little bit excesive on the local variables
void projectToTile (CompGeom::Tile& T, const CompGeom::Geometry & geom, 
		    vector<float> ex, const size_t & dir ) 
{
  const size_t w   = T.nCols();
  const size_t h   = T.nRows();
  const size_t i   = dir  %DIM;
  const size_t j   = (i+1)%DIM;
  const size_t k   = (i+2)%DIM;
  const float minw = ex[j];
  const float minh = ex[k];
  const float maxw = ex[j+DIM];
  const float maxh = ex[k+DIM];

  for ( size_t p_i = 0; p_i<geom.size(); p_i++ ) {
    const auto &p = geom[p_i];
    int	idj = int(w*(p[j]-minw)/((maxw-minw)*(1+EPS)));
    int	idk = int(h*(p[k]-minh)/((maxh-minh)*(1+EPS)));    

    float	cmin_val = T.get(idj,idk);
    float	this_min = fabs(p[i]-ex[i]);
    if ( cmin_val > this_min ) {
      T.set  (idj,idk, this_min   );
      T.setID(idj,idk, p_i        );
    }
  }
}

// Step 1
// Divide the box into bricks containing chunks of the data. 
// Project the points onto tiles on the faces
// deciding conflicts by choosing the closer point
void projectToBox ( CompGeom::BoundingBox &B, const CompGeom::Geometry &geom ) {
  vector < float > extremes = findExtremes2 ( geom );

  for ( auto dir : Direction::allDirections() ) { 
    projectToTile ( B[dir], geom, extremes, dir );
  }
}

//////////////////////// Construct Voronois ////////////////////////////////

inline size_t shortID ( int i, int j , int w) {
  i = (i+w)%w;
  j = (j+w)%w;
  return 2*(i*w + j);
}


// Converts our the tile of projections into pairs of short indexes
void fillVoronoiInput ( short * input, const CompGeom::Tile &T ) {
  size_t	w = T.nCols();
  size_t	h = T.nRows();

  std::fill ( input, input + 2*w*h, MARKER );
  for ( size_t idj = 0; idj < w; idj++ ) {
    for ( size_t idk = 0; idk < h; idk++ ) {
      int index = shortID(idj,idk,w);    
      if ( T.get(idj,idk) != numeric_limits<float>::max() ) {
	input[index]	  = idk;
	input[index+1]	  = idj;
      }
    }
  }
}

// Prints the array of pairs of short indexes
void  printDiagram ( short * input, size_t w , size_t h) {
  for ( size_t l=0; l<h; l++ ) {
    for ( size_t j	 = 0; j<w; j++ ) {
      int	index = shortID(l,j,w);
      if ( input[index] == MARKER ) cout << "- ";
      else cout << "(" << input[index] << "," << input[index+1] << ") ";
    }
    cout << endl;
  }
  cout << endl << endl;
}


// This function constructs Voronois on the boxes projections
void constructVoronois ( CompGeom::BoundingBox & B, vector < Voronoi > &VD) {
  using namespace Direction;

  size_t	w = B.width();
  size_t	h = B.height();
  
  // Initialise the memory to be passed to pba
  vector < short > input ( 2*w*h );
  vector < short > output ( 2*w*h );

  // Allocates memory for the pba library
  // I'm unsure what the parameter is that we are passing
  // But in the example main, this is what is passed for
  // an input array of size 2*w*w as we have here.
  pba2DInitialization(w);

  // Voronoi is the same on opposite sides
  // So it is only calculated on the bottom 4 tiles
  for ( auto dir : { LEFT, BACK, DOWN } ) {
    // Calculate Voronoi diagram on min tile
    fillVoronoiInput   (&input[0],B[dir]                );
    pba2DVoronoiDiagram(&input[0],&output[0],P1B,P2B,P3B);

    // Print Voronoi to file
    // string filename  = "images/voronoi" + to_string(dir) + ".pbm";
    // makeVoronoiPBM(output,input,w,h,filename);

    auto & V = VD[dir];
    for ( size_t i   = 0; i<h; i++ ) {
      for ( size_t j = 0; j<w; j++ ) {
	size_t	index = shortID(i,j,w);
	copy_n ( &output[index], 2, V[i][j].begin() );
      }
    }
  }

  // Free all memory
  pba2DDeinitialization();
}

// inner if statement would probably be faster the other way around
void findDualEdges ( vector<OrderedEdge> &W, const Tile &T, const Voronoi & V ) {
  int L = T.length();

  for ( int i=0; i<L; i++ ) {
    for ( int j=0; j<L; j++ ) {

      // Direction of the nearest neighbours
      vector<vector<int>> dir =  {{-1,0},{0 ,-1},{0 ,1},{1 ,0}};      
      for ( auto d : dir ) {
	int ni = i+d[0];
	int nj = j+d[1];
	if ( !(ni < 0 || nj < 0 || ni >= L || nj >= L) 
	     && V[ni][nj] != V[i][j] ) 
	  {
	    // pba2D has indexing (j,i) so the ids are swapped around
	    size_t id0 = T.getID ( V[i ][j ][1], V[i ][j ][0] );
	    size_t id1 = T.getID ( V[ni][nj][1], V[ni][nj][0] );
	    W.push_back(OrderedEdge ( id0, id1 ));
	  }
      }
    }
  }
}

void constructWorkingSets ( vector<WorkingSet> & W, 
			    const BoundingBox & B, 
			    const vector < Voronoi > & V ) 
{
  vector < OrderedEdge > Vedges;
  
  // Find all the edges from the Voronoi diagrams 
  for ( auto dir : Direction::allDirections() ) {
    findDualEdges ( Vedges, B[dir], V[dir%3] );
  }

  // Sort and remove duplicates
  sort(Vedges.begin(),Vedges.end());
  auto it = unique(Vedges.begin(),Vedges.end());
  Vedges.resize ( distance ( Vedges.begin(), it ), OrderedEdge(-1,-1) );

  // Construct the vector of Working Sets
  size_t curr_index = 0;
  WorkingSet curr;
  for ( auto ei : Vedges ) {
    if ( ei.first == curr_index ) {
      curr.push_back(ei);
    } 
    else {
      curr_index = ei.first;
      W.push_back(curr);
      curr.resize(0, OrderedEdge(-1,-1));
    }
  }
}

WorkingSet constructStar_h ( const WorkingSet & W ) {
  if ( W.size() < 3 ) errorM("Working Set does not have enough edges");

  for ( auto ei : W ) {
    ;;
  }
  return WorkingSet(0,OrderedEdge(-1,-1));
}

void constructStars       ( vector < WorkingSet >& S, const vector < WorkingSet > &W ) {
  for ( auto& wset : W ) {
    S.push_back(constructStar_h ( wset ));
  }
}

vector < vector < size_t > > gHullSerial ( const CompGeom::Geometry &geom ) {
  if ( geom.size()    < 4 ) errorM("3D geometry needs at least for non-coplanar points to be a convex hull"); 
  if ( geom.getDim() != 3 ) errorM("gHullSerial only works in 3 dimensions");

  CompGeom::BoundingBox  B  (BOXSIZE );
  vector < Voronoi     > V  (DIM    ,Voronoi(BOXSIZE, VoronoiRow(BOXSIZE, vector < short > (2) ) ) );
  // vector < OrderedEdge > W  ;
  vector < WorkingSet  > W, S; 

  projectToBox         ( B, geom );
  constructVoronois    ( B, V    );
  constructWorkingSets ( W, B, V );
  constructStars       ( S, W    );

  // makeVoronoiPBM(V[0],"images/test1.pbm");
  // makeVoronoiPBM(V[1],"images/test2.pbm");
  // makeVoronoiPBM(V[2],"images/test3.pbm");
  
  return vector < vector < size_t > > ( 1,{0,1,2} );
}

