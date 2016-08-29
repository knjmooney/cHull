/******************************************************
 * Name    : edge.hpp
 * Author  : Kevin Mooney
 * Created : 20/07/16
 * Updated : 
 *
 * Description:
 *   a lightweight pair class with an == operator 
 *   that compares reflections of edges
 *
 * ToDo:
 ******************************************************/

#pragma once

#include <limits>

#include "errorMessages.hpp"

namespace CompGeom {
  
  struct Edge {
    
    size_t first = std::numeric_limits<size_t>::max();
    size_t second = std::numeric_limits<size_t>::max();
    
    // Store in sorted order
    Edge ( size_t id0, size_t id1 ) : first{id0}, second{id1} {}
  };
}



