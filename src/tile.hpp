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

#include <algorithm>
#include <cstdio>
#include <limits>

#include "directionEnums.hpp"

namespace CompGeom {
  class Tile {
  private:
    std::vector < float  > _VF;
    std::vector < size_t > _VIDS;
    const size_t	   _length;

    float  maxfloat() const { return std::numeric_limits<float>::max(); }
    size_t maxID()    const { return std::numeric_limits<size_t>::max(); }
    size_t index(const size_t &i, const size_t &j) const {return j + i*_length;}

  public:
    Tile ( size_t length )
      : _VF  {std::vector<float >(length*length, maxfloat())} 
      , _VIDS{std::vector<size_t>(length*length, maxID()   )}
      , _length{length}
    {}

    float  get   ( size_t i, size_t j		) const	{ return _VF[index(i,j)];	}
    size_t getID ( size_t i, size_t j		) const	{ return _VIDS[index(i,j)];	}
    void   set   ( size_t i, size_t j, float val)	{ _VF  [index(i,j)] = val;	}
    void   setID ( size_t i, size_t j, size_t id)	{ _VIDS[index(i,j)] = id;	}

    size_t length	() const { return _length;	}
    size_t nCols	() const { return _length;	}
    size_t nRows	() const { return _length;	}

    void print() const {
      for ( size_t i=0; i<_length; i++ ) {
	for ( size_t j=0; j<_length; j++ ) {
	  size_t	id = index(i,j);
	  if ( _VF[id] == maxfloat() ) printf(" -  "         ); 
	  else                       printf("%3.1f ",_VF[id]);
	}
	printf("\n"); fflush(stdout);
      }
    }
  };
}
