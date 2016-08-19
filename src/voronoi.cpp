/******************************************************
 * Name    : voronoi.cpp
 * Author  : Kevin Mooney
 * Created : 12/08/16
 * Updated :
 *
 * Description:
 *
 * NOTES:
 ******************************************************/

#include "pba2D.h"
#include "voronoi.hpp"

using namespace std;

// Why does this have to be static???
// Compains of multiple definition with function
// in convexHull3D.hpp
static int shortID ( int i, int j , int w) {
  i = (i+w)%w;
  j = (j+w)%w;
  return 2*(i*w + j);
}

// Returns true if any nearest neighbour is not set to MARKER
bool isNearestNeighbourOn( short *V, size_t i, size_t j,size_t w ) {
  vector<vector<int>> dir = {{-1,-1},{-1,0},{-1,1},
			     {0 ,-1},{0 ,0},{0 ,1},
			     {1 ,-1},{1 ,0},{1 ,1}};
  for ( auto d : dir ) {
    int ii = i+d[0], jj = j+d[1]; 
    int index = shortID(ii,jj,w) ;
    if ( V[index] != MARKER ) {
      return true;
    }
  }
  return false;
}

// Stolen from Rosetta Code
// Creates a portable bitmap file with a Voronoi diagram
void makeVoronoiPBM( short * V, short * In, size_t w, size_t h, const string &filename ) {  
  size_t i, j;
  FILE *fp = fopen(filename.c_str(), "wb"); /* b - binary mode */
  fprintf(fp, "P6\n%lu %lu\n255\n", w, h);
  for (i = 0; i < w; ++i) {
    for (j = 0; j < h; ++j) {
      int index = shortID(i,j,w);
      static unsigned char color[3];
      if ( isNearestNeighbourOn(In,i,j,w) ) {
	color[0] = color[1] = color[2] = 0;
      }
      else {
	srand(V[index]*V[index+1]);
	color[0] = rand()   % 256;  /* red */
	color[1] = rand()   % 256;  /* green */
	color[2] = rand()   % 256;  /* blue */
      }
      (void) fwrite(color, 1, 3, fp);
    }
  }
  fclose(fp);
}


void makeVoronoiPBM( const Voronoi &V, const string &filename ) {  
  FILE *fp = fopen(filename.c_str(), "wb"); /* b - binary mode */
  fprintf(fp, "P6\n%lu %lu\n255\n", V.size(), V.size());
  for ( auto row : V ) {
    for ( auto p : row ) {
      static unsigned char color[3];
      srand(p[0]*p[1]);
      color[0] = rand()   % 256;  /* red */
      color[1] = rand()   % 256;  /* green */
      color[2] = rand()   % 256;  /* blue */
      (void) fwrite(color, 1, 3, fp);
    }
  }
  fclose(fp);
}

