/******************************************************
 * Name    : boundingBox.hpp
 * Author  : Kevin Mooney
 * Created : 25/07/16
 * Updated :
 *
 * Description:
 *
 * NOTES:
 ******************************************************/

#include "tile.hpp"

namespace CompGeom {
  class BoundingBox {
  private:
    Tile ** _T;
  public:
    BoundingBox ( size_t w, size_t h ) : _T{new Tile*[6]} {
      for ( int i=0; i<6; i++ ) {
	_T[i] = new Tile(w,h);
      }
    }
    ~BoundingBox () { 
      for ( int i=0; i<6; i++ ) {
      	delete _T[i];
      }    
      delete [] _T; 
    }

    Tile * up() {
      return _T[0];
    }
    Tile * down() {
      return _T[1];
    }
    Tile * left() {
      return _T[2];
    }
    Tile * right() {
      return _T[3];
    }
    Tile * back() {
      return _T[4];
    }
    Tile * front() {
      return _T[5];
    }
  };
}
