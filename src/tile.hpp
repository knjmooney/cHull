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

#include <limits>


namespace CompGeom {
  class Tile {
  private:
    float *_F;
    size_t _width, _height;
    size_t index(size_t i, size_t j) {return i+j*_width;}
  public:
    Tile () : _F{NULL}, _width{0}, _height{0} {}
    Tile ( size_t w, size_t h ) : _F{new float[w*h]}, _width{w}, _height{h} {
      for ( size_t i = 0; i<w*h; i++ ) { _F[i] = std::numeric_limits<float>::max(); }
    }
    ~Tile () { delete _F; }

    float get(size_t i, size_t j) {
      return _F[i + j*_width];
    }

    float width() {
      return _width;
    }
    
    float height() {
      return _height;
    }

    void set (size_t i, size_t j, float val) {
      if ( i >= _height || j >= _width ) { throw std::out_of_range("Indexes are out of range"); }
      _F[i+j*_width] = val;
    }
  };
}
