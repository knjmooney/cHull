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
#include <list>
#include <set>
#include <string>
#include <vector>

#include "boundingBox.hpp"
#include "convexHull3D.hpp"
#include "geometry.hpp"
#include "geometryHelper.hpp"
#include "orderedEdge.hpp"
#include "unorderedEdge.hpp"
#include "pba2D.h"		// Parallel Banding Algorithm
#include "triangle.hpp"
#include "voronoi.hpp"
#include "workingSet.hpp"
#include "star.hpp"
#include "starHull.hpp"

#include "removeInsert.hpp"

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
    float	this_min = fabs(p[i]-ex[dir]);
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

// Finds the dual of the Voronoi diagram
void findDualEdges ( vector<OrderedEdge> &W, const Tile &T, const Voronoi & V ) {
  int L = T.length();

  for ( int i=0; i<L; i++ ) {
    for ( int j=0; j<L; j++ ) {

      // Direction of the nearest neighbours
      const vector<vector<int>> dir =  {{-1,0},{0 ,-1},{0 ,1},{1 ,0}};      
      for ( auto d : dir ) {
	int ni = i+d[0];
	int nj = j+d[1];

	// Check bounds first
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

  // Sort and remove duplicates
  sort(Vedges.begin(),Vedges.end());
  auto it = unique(Vedges.begin(),Vedges.end());
  Vedges.resize ( distance ( Vedges.begin(), it ), OrderedEdge(-1,-1) );

  // cout << "====== SERIAL ======\n";
  cout << "Serial Size : " << Vedges.size() << endl;
  // for ( auto edge : Vedges ) {
  //   cout << edge.first << " " << edge.second << endl;
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

// Untested
void delete_visible_backwards ( list<Triangle> &Star, const CompGeom::Point &p ) {
  auto it = Star.end();
  --it;
  while ( it->isVisible(p) ) {
    --it;
  }
  ++it;
  Star.erase(it,Star.end());
}

// Untested
void delete_visible_forwards ( list<Triangle> &Star, const CompGeom::Point &p) {
  auto it = Star.begin();
  while ( it->isVisible(p) ) {
    it++;
  }
  Star.erase(Star.begin(),it);
}

// Rethink checking the start is at the end, initial if statement
template <typename iter>
iter findNextVisible    (iter start,iter end,const size_t &star_id, const size_t &id,const Geometry & geom) {
  iter second = start;
  advance(second,1);
  const Point &p = geom[id];
  for ( ; start != end && second != end; ++start, ++second ) {
    Triangle tri ( star_id, *start, *second, geom );
    if ( tri.isVisible(p) )
      return start;
  }
  return end;
}

// Rethink checking the start is at the end, initial if statement
template <typename iter>
iter findNextNotVisible    (iter start,iter end,const size_t &star_id, const size_t &id,const Geometry & geom) {
  iter second = start;
  advance(second,1);
  const Point &p = geom[id];
  for ( ; start != end && second != end ; ++start, ++second ) {
    Triangle tri ( star_id, *start, *second, geom );
    if ( !tri.isVisible(p) )
      return start;
  }
  return end;
}

Star constructStar_h ( const WorkingSet & W, const CompGeom::Geometry &geom ) {
  if ( W.size() < 3 ) errorM("Working Set does not have enough edges");

  Star tstar(W[0].first);
  Triangle t0( tstar.id , W[0].second, W[1].second, geom );

  auto tid = W[2].second;	// temp id
  if ( t0.isVisible(geom[tid]) ) {
    t0.invert();
  }

  tstar.insert( tstar.end(), t0[1] );
  tstar.insert( tstar.end(), t0[2] );
  tstar.insert( tstar.end(), tid    );

  auto it = W.begin();
  for ( advance(it,3); it!=W.end(); it++ ) {
    const auto &pid = it->second; // point id
    const auto &p  = geom[pid];
    
    // If the edge from the back to the front of the list is visible
    Triangle tri(tstar.id,tstar.back(),tstar.front(),geom);
    if ( tri.isVisible(p) ) {      
      Star::iterator to   = findNextNotVisible (tstar.begin(), tstar.end(), tstar.id, pid, geom);
      to = tstar.erase(tstar.begin(),to);
      if ( tstar.empty() ) {	// If all edges are visible then the star is dead
	break;
      }

      Star::iterator from = findNextVisible    (to           , tstar.end(), tstar.id, pid, geom);
      if ( from != tstar.end() ) {     // If there's nothing to delete
	advance(from,1);	       // don't delete the first vertex
	tstar.erase(from,tstar.end()); // be careful if to and from overlap (should be impossible)
      }
      tstar.insert(tstar.end(),pid);
    }
    else {
      Star::iterator from = findNextVisible    (tstar.begin(),tstar.end(), tstar.id, pid, geom);
      Star::iterator to   = findNextNotVisible (from         ,tstar.end(), tstar.id, pid, geom);
      if ( from == tstar.end() ) continue;			   // If there's nothing to delete

      advance(from,1);		    // don't delete the first vertex
      to = tstar.erase ( from,to ); // be careful if from==to (I don't think this happens)
      tstar.insert(to,pid);
    }
  }

  // ch.print ( "data/test.dat" );
  return tstar;
}


void constructStars   ( vector < Star >& S, 
			const vector < WorkingSet > &W, 
			const CompGeom::Geometry &geom ) 
{
  // StarHull shull(geom);
  for ( auto& wset : W ) {
    // try { 
    Star tstar = constructStar_h ( wset, geom );
    if ( !tstar.empty() ) {
      S.push_back(tstar);
    }
    // shull.update(S.begin(),S.end());
    
    // } catch( std::logic_error &e ) {
    //   cout << __LINE__ << " " << e.what() << endl;
    //   for ( auto ei : wset ) 
    // 	cout << ei.first <<" " << ei.second << endl;
    // }
  }
  // shull.print("test_starset.txt");
}

vector < vector < size_t > > gHullSerial ( const CompGeom::Geometry &geom ) {
  if ( geom.size()    < 4 ) errorM("3D geometry needs at least for non-coplanar points to be a convex hull"); 
  if ( geom.getDim() != 3 ) errorM("gHullSerial only works in 3 dimensions");

  CompGeom::BoundingBox  B  (BOXSIZE );
  vector < Voronoi     > V  (DIM    ,Voronoi(BOXSIZE, VoronoiRow(BOXSIZE, vector < short > (2) ) ) );
  vector < WorkingSet  > W; 
  vector < Star        > S;

  projectToBox         ( B, geom    );
  constructVoronois    ( B, V       );
  constructWorkingSets ( W, B, V    );
  constructStars       ( S, W, geom );

  // makeVoronoiPBM(V[Direction::LEFT],"images/voronoi_left.pbm" ,B[Direction::LEFT ]);
  // makeVoronoiPBM(V[Direction::BACK],"images/voronoi_back.pbm" ,B[Direction::BACK ]);
  // makeVoronoiPBM(V[Direction::DOWN],"images/voronoi_down.pbm" ,B[Direction::DOWN ]);
  // makeVoronoiPBM(V[Direction::LEFT],"images/voronoi_right.pbm",B[Direction::RIGHT]);
  // makeVoronoiPBM(V[Direction::BACK],"images/voronoi_front.pbm",B[Direction::FRONT]);
  // makeVoronoiPBM(V[Direction::DOWN],"images/voronoi_up.pbm"   ,B[Direction::UP   ]);

  
  return vector < vector < size_t > > ( 1,{0,1,2} );
}

