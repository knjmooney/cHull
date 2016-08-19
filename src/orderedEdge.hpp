/******************************************************
 * Name    : OrderedEdge.hpp
 * Author  : Kevin Mooney
 * Created : 20/07/16
 * Updated : 
 *
 * Description:
 *   a lightweight pair class with an == operator 
 *   that compares reflections of OrderedEdges
 *
 * ToDo:
 ******************************************************/

#pragma once

#include "edge.hpp"
#include "errorMessages.hpp"

namespace CompGeom {
  
  struct OrderedEdge : public Edge {
    
    // Store in sorted order
    OrderedEdge ( size_t id0, size_t id1 ) : Edge{id0,id1} {}    
  };

  inline bool operator==(const OrderedEdge &e0, const OrderedEdge &e1) {
    return (e0.first==e1.first && e0.second==e1.second);
  }

  inline bool operator< (const OrderedEdge &e0, const OrderedEdge &e1) {
    if ( e0.first == e1.first )  return e0.second < e1.second;
    else                         return e0.first  < e1.first ;
  }
}


