/******************************************************
 * Name    : boundingBox.hpp
 * Author  : Kevin Mooney
 * Created : 25/07/16
 * Updated : 03/08/16
 *
 * Description:
 *
 * NOTES:
 *  -_T is a double pointer to avoid having to call 
 *   the default constructor, there must be a better 
 *   way to do this
 *  - I originally wrote this with using the class in 
 *    CUDA too, this was a silly idea
 ******************************************************/

#pragma once

#include <vector>

#include "directionEnums.hpp"
#include "geometry.hpp"
#include "tile.hpp"

namespace CompGeom {

  class BoundingBox {
  private:
    std::vector < Tile > _T;
    const size_t  _length;	// width, height and depth
  public:
    BoundingBox ( const size_t &w );

    size_t length() const { return _length; }    

    Tile& operator[](enum Direction::Dir i) { return _T[i]; }
    const Tile& operator[](enum Direction::Dir i) const { return _T[i]; }

    // Depreciated
    size_t height() const { return _length; } 
    size_t width()  const { return _length; }
  };
}
