/******************************************************
 * Name    : UnorderedEdge.hpp
 * Author  : Kevin Mooney
 * Created : 20/07/16
 * Updated : 
 *
 * Description:
 *   a lightweight pair class with an == operator 
 *   that compares reflections of UnorderedEdges
 *
 * ToDo:
 ******************************************************/

#pragma once

#include "edge.hpp"
#include "errorMessages.hpp"

namespace CompGeom {
  
  struct UnorderedEdge : public Edge {
    
    // Store in sorted order
    UnorderedEdge ( size_t id0, size_t id1 ) : Edge{id0,id1} {}
    // ~UnorderedEdge() {}    
  };

  inline bool operator==(const UnorderedEdge &e0, const UnorderedEdge &e1) {
    return (e0.first==e1.first && e0.second==e1.second) ||
           (e0.second==e1.first && e0.first==e1.second);
  }
}


