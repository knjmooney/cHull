/******************************************************
 * Name    : tile.hpp
 * Author  : Kevin Mooney
 * Created : 25/07/16
 * Updated :
 *
 * Description:
 *
 * NOTES:
 ******************************************************/

#pragma once

#include <cstdio>
#include <limits>
// #include <pba2D.h>

namespace CompGeom {
  class Tile {
  private:
    float *_F;
    const size_t _width, _height;
    const float  maxf = std::numeric_limits<float>::max();

    size_t index(size_t i, size_t j) {return i+j*_width;}
  public:
    Tile () : _F{NULL}, _width{0}, _height{0} {}
    Tile ( size_t h, size_t w ) : _F{new float[w*h]}, _width{w}, _height{h} {
      std::fill ( _F, _F+w*h, maxf );
    }
    ~Tile () { delete[] _F; }

    float get(size_t i, size_t j) {
      return _F[j + i*_width];
    }

    float width() {
      return _width;
    }
    
    float height() {
      return _height;
    }

    void set (size_t i, size_t j, float val) {
      if ( i >= _height || j >= _width ) { throw std::out_of_range("Indexes are out of range"); }
      _F[j+i*_width] = val;
    }

    void print() {
      for ( size_t i=0; i<_height; i++ ) {
	for ( size_t j=0; j<_width; j++ ) {
	  size_t id = index(i,j);
	  if ( _F[id] == maxf ) printf(" -  "         ); 
	  else                  printf("%3.1f ",_F[id]);
	}
	printf("\n"); fflush(stdout);
      }
    }
  };
}
