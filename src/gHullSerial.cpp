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
#include "triangle.hpp"
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

// I should reconsider using two namespaces
using namespace std;
using namespace CompGeom;

//////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  GHULL SERIAL  /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

// Maybe a little bit excesive on the local variables
void projectToTile (CompGeom::Tile& T, const CompGeom::Geometry & geom, 
		    vector<float> ex, const Direction::Dir dir ) 
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


  // int watchj=-1, watchk=-1;
  for ( size_t p_i = 0; p_i<geom.size(); p_i++ ) {
    const auto &p = geom[p_i];
    int	idj = int(w*(p[j]-minw)/((maxw-minw)*(1+EPS)));
    int	idk = int(h*(p[k]-minh)/((maxh-minh)*(1+EPS)));    

    float	cmin_val = T.get(idj,idk);
    float	this_min = fabs(p[i]-ex[i]);
    if ( cmin_val > this_min ) {
      // if ( p_i == 101 || ( watchj == idj && watchk == idk ) ) {
      // 	cout << dir << " " << p_i << " " << idj << " " << idk <<  endl;
      // 	watchj = idj; watchk = idk;
      // }
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

  // int count = 0;
  std::fill ( input, input + 2*w*h, MARKER );
  for ( size_t idj = 0; idj < w; idj++ ) {
    for ( size_t idk = 0; idk < h; idk++ ) {
      int index = shortID(idj,idk,w);    
      if ( T.get(idj,idk) != numeric_limits<float>::max() ) {
	input[index]	  = idk;
	input[index+1]	  = idj;
	// count++;
      }
    }
  }
  // cout << count << endl;
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
	    // if ( id0 == 101 ) cout << id0 << " " << id1 << endl;
	    // if ( id1 == 101 ) cout << id0 << " " << id1 << endl;
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

  // vector < OrderedEdge > temp;
  // for ( auto row : V[2] ) {
  //   for ( auto ei : row ) {
  //     temp.push_back( OrderedEdge ( ei[0], ei[1] ) );
  //   }
  // }
  // sort ( temp.begin(), temp.end() );
  // auto end_it = unique ( temp.begin(), temp.end() );
  // cout << "temp distance: " << distance ( temp.begin(), end_it ) << endl;


  // Sort and remove duplicates
  sort(Vedges.begin(),Vedges.end());
  auto it = unique(Vedges.begin(),Vedges.end());
  Vedges.resize ( distance ( Vedges.begin(), it ), OrderedEdge(-1,-1) );

  // for ( auto ei : Vedges ) {
  //   if ( ei.first == 101 ) cout << ei.first << " " << ei.second << endl;
  // }

  // Construct the vector of Working Sets
  size_t curr_index = 0;
  WorkingSet curr;
  for ( auto ei : Vedges ) {
    if ( ei.first != curr_index ) {
      if ( !curr.empty() ) W.push_back(curr);
      curr.resize(0, OrderedEdge(-1,-1));
      curr_index = ei.first;
    }
    curr.push_back(ei);
  }
}

WorkingSet constructStar_h ( const WorkingSet & W, const CompGeom::Geometry &geom ) {
  if ( W.size() < 3 ) errorM("Working Set does not have enough edges");

  Triangle ( W[0].first, W[0].second, W[1].second, geom );

  // for ( auto ei : W ) {
  //   ;;
  // }
  return WorkingSet(0,OrderedEdge(-1,-1));
}

void constructStars   ( vector < WorkingSet >& S, 
			const vector < WorkingSet > &W, 
			const CompGeom::Geometry &geom ) 
{
  for ( auto& wset : W ) {
    // try { 
    S.push_back(constructStar_h ( wset, geom ));
    // } catch( std::logic_error &e ) {
    //   cout << __LINE__ << " " << e.what() << endl;
    //   for ( auto ei : wset ) 
    // 	cout << ei.first <<" " << ei.second << endl;
    // }
  }
}

vector < vector < size_t > > gHullSerial ( const CompGeom::Geometry &geom ) {
  if ( geom.size()    < 4 ) errorM("3D geometry needs at least for non-coplanar points to be a convex hull"); 
  if ( geom.getDim() != 3 ) errorM("gHullSerial only works in 3 dimensions");

  CompGeom::BoundingBox  B  (BOXSIZE );
  vector < Voronoi     > V  (DIM    ,Voronoi(BOXSIZE, VoronoiRow(BOXSIZE, vector < short > (2) ) ) );
  vector < WorkingSet  > W, S; 

  projectToBox         ( B, geom    );
  constructVoronois    ( B, V       );
  constructWorkingSets ( W, B, V    );
  constructStars       ( S, W, geom );

  makeVoronoiPBM(V[0],"images/test1.pbm");
  makeVoronoiPBM(V[1],"images/test2.pbm");
  makeVoronoiPBM(V[2],"images/test3.pbm");
  
  return vector < vector < size_t > > ( 1,{0,1,2} );
}

