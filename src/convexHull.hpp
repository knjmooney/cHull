/******************************************************
 * Name    : convexHull.hpp
 * Author  : Kevin Mooney
 * Created : 27/06/16
 * Updated : 
 *
 * Description:
 *
 * ToDo:
 ******************************************************/

#pragma once

#include "geometry.hpp"


namespace CompGeom {

  // Should I inherit publically?
  class ConvexHull : public Geometry {
    ConvexHull ( const size_t &_dim) : Geometry(_dim) {}
    
  };

}
