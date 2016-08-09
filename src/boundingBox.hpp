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

#include "tile.hpp"

namespace CompGeom {
  class BoundingBox {
  private:
    Tile ** _T;
    size_t _w, _h;
  public:
    BoundingBox ( size_t w, size_t h ) : _T{new Tile*[6]}, _w{w}, _h{h} {
      for ( int i=0; i<6; i++ ) {
	_T[i] = new Tile(w,h);
      }
    }
    ~BoundingBox () { 
      for ( int i=0; i<6; i++ ) {
      	delete _T[i];
      }    
      delete[] _T; 
    }

    size_t height() {
      return _h;
    }

    size_t width() {
      return _w;
    }

    Tile * up() const {
      return _T[0];
    }
    Tile * down() const {
      return _T[1];
    }
    Tile * left() const {
      return _T[2];
    }
    Tile * right() const {
      return _T[3];
    }
    Tile * back() const {
      return _T[4];
    }
    Tile * front() const {
      return _T[5];
    }

  };
}
