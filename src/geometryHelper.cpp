/******************************************************
 * Name    : geometryHelper.cpp
 * Author  : Kevin Mooney
 * Created : 16/08/16
 * Updated :
 *
 * Description:
 *
 * NOTES:
 ******************************************************/

#include <vector>

#include "directionEnums.hpp"
#include "geometry.hpp"

// Finds the minimum and maximum coordinates in all dimensions
// This could be reduced to stl functions with lambdas
// check minmax_element
std::vector < float > findExtremes2 ( const CompGeom::Geometry &geom ) {
  using namespace Direction;
  std::vector < float > ext(6);
  
  // Set initial values for the mins and maxes
  // for ( int i=0; i<3; i++ ) {
  for ( auto dir : Direction::allDirections() ) { 
    size_t i = dir % 3;
    ext[i] = geom[0][i];
  }
  // Loop over all points and update min or max if necessary
  for ( auto p : geom ) {
    for ( auto dir : {LEFT, DOWN, BACK } ) { 
      size_t i = dir % 3;
      ext[dir] = std::min(ext[dir],p[i]);
    }
    for ( auto dir : {RIGHT, UP, FRONT } ) { 
      size_t i = dir % 3;
      ext[dir] = std::max(ext[dir],p[i]);
    }  
  }  
  return ext;
}
