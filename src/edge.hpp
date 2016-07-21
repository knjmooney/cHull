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

#include "errorMessages.hpp"

namespace CompGeom {
  
  struct Edge {
    
    size_t first;
    size_t second;
    
    // Store in sorted order
    Edge ( size_t id0, size_t id1 ) : first{id0}, second{id1} {}
    ~Edge() {}
    
  };

  inline bool operator==(const Edge &e0, const Edge &e1) {
    return (e0.first==e1.first && e0.second==e1.second) ||
           (e0.second==e1.first && e0.first==e1.second);
  }
}


