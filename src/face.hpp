/******************************************************
 * Name    : face.hpp
 * Author  : Kevin Mooney
 * Created : 17/07/16
 * Updated : 
 *
 * Description:
 *
 * ToDo:
 *   com and norm could be stored as precalculated values
 ******************************************************/

#pragma once

#include "errorMessages.hpp"
#include "point.hpp"

namespace CompGeom {
  
  class Face {
    
  private:
    std::vector<Point> _v;
    
  public:
    Face ( std::vector<Point> verts) : _v{verts} { 
      if(_v.size() != 3) errorM("A face must contain 3 points");
    }
    Face ( std::initializer_list<Point> verts ) : _v{verts} {}    
    ~Face() {}
    
    // Operator overloading
    Point operator   [](int i) const  {return _v[i];}
    Point &operator  [](int i)        {return _v[i];}

    // Points are stored in anti-clockwise order when viewing from the norm
    Point normal () {
      Point x ( _v[1] - _v[0] ), y ( _v[2] - _v[0] );
      return Point { { x[1]*y[2] - x[2]*y[1], x[2]*y[0] - x[0]*y[2], x[0]*y[1] - x[1]*y[0] } };
    }
    
    // Calculates the centre of mass of a face
    Point com () {
      return Point { { (_v[0][0] + _v[1][0] + _v[2][0])/3, 
	               (_v[0][1] + _v[1][1] + _v[2][1])/3, 
	               (_v[0][2] + _v[1][2] + _v[2][2])/3 } };
    }

    // Checks if point is visible from the face
    bool isVisible ( Point p ) {
      Point n { normal() };
      Point &v = _v[0];
      return n[0]*(p[0] - v[0]) + n[1]*(p[1] - v[1]) + n[2]*(p[2] - v[2]) > 0;
    }

    void invert () {
      std::swap ( _v[1], _v[2] );
    }
  };
}


