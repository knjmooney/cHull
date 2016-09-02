/******************************************************
 * Name    : triangle.hpp
 * Author  : Kevin Mooney
 * Created : 17/07/16
 * Updated : 
 *
 * Description:
 *   Rewrite of face.hpp, stores indexes of vertices
 *   corresponding to geometry as well as norm, com 
 *   and offset d
 *
 *   Equation of plane is n0*x0 + n1*x1 + n2*x2 + d = 0
 * ToDo:
 ******************************************************/

#pragma once

#include <algorithm>
#include <vector>

#include "errorMessages.hpp"
#include "point.hpp"

namespace CompGeom {
  
  class Triangle {
    
  private:
    std::vector<size_t> _vertices;
    Point _norm;
    Point _com;

  public:
    // The norm and centre of mass are calculated on construction 
    // and then all points are discarded except for their IDS
    Triangle ( size_t id0, size_t id1, size_t id2, const Geometry &geom) : 
      _vertices{{id0,id1,id2}}, 
      _norm{Point(3)}, 
      _com {Point(3)} 
    { 

      const Point &p0 = geom[id0], &p1 = geom[id1], &p2 = geom[id2];
      _com  = Point { { (p0[0] + p1[0] + p2[0])/3, 
      			(p0[1] + p1[1] + p2[1])/3, 
      			(p0[2] + p1[2] + p2[2])/3 } };

      Point x ( p1 - p0 ), y ( p2 - p0 );
      _norm = Point { { x[1]*y[2] - x[2]*y[1], x[2]*y[0] - x[0]*y[2], x[0]*y[1] - x[1]*y[0] } };
    }
    ~Triangle() {}
    
    // Operator overloading
    size_t operator   [](int i) const  {return _vertices[i];}

    // Points are stored in anti-clockwise order when viewing from the norm
    inline Point normal () const {
      return _norm;
    }
    
    // Return centre of mass
    inline Point com () const {
      return _com;
    }

    // Checks if point is visible from the triangle
    bool isVisible ( Point p ) const  {
      const Point &n = _norm;
      const Point &v = _com;
      return n[0]*(p[0] - v[0]) + n[1]*(p[1] - v[1]) + n[2]*(p[2] - v[2]) > 0;
    }

    void invert () {
      std::swap ( _vertices[1], _vertices[2] );
      _norm = - _norm;		// This might be inefficient
    }
    
  };

  inline bool operator< ( Triangle ti, Triangle tj ) {
    std::vector<size_t> temp1 = { ti[0], ti[1], ti[2] };
    std::vector<size_t> temp2 = { tj[0], tj[1], tj[2] };
    sort ( temp1.begin(), temp1.end() );
    sort ( temp2.begin(), temp2.end() );

    return temp1 < temp2;
  }

  inline bool operator== ( Triangle ti, Triangle tj ) {
    return (ti[0] == tj[0] && ti[1] == tj[1] && ti[2] == tj[2]) || 
           (ti[0] == tj[1] && ti[1] == tj[2] && ti[2] == tj[0]) ||
           (ti[0] == tj[2] && ti[1] == tj[0] && ti[2] == tj[1]);
      
  }
}


