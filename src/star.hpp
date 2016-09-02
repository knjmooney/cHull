/******************************************************
 * Name    : star.hpp
 * Author  : Kevin Mooney
 * Created : 18/08/16
 * Updated : 
 *
 * Description:
 *
 * NOTES:
 ******************************************************/

#pragma once

#include <vector>

#include "workingSet.hpp"

namespace CompGeom {

  class Star {
  private:
    typedef std::vector<size_t> container;
    
  public:
    size_t id;
    std::vector<size_t> edges;
    
    Star ( const size_t &id ) : id{id} {}

    typedef typename std::vector<size_t>::iterator iterator;
    typedef typename std::vector<size_t>::const_iterator const_iterator;
    iterator       begin()        { return edges.begin(); }
    iterator       end()          { return edges.end();   }
    const_iterator begin() const  { return edges.begin(); }
    const_iterator end()   const  { return edges.end();   }
    
    size_t front() const { return edges.front(); }
    size_t back()  const { return edges.back (); }

    void     insert ( const iterator &it, const size_t &id     ) {        edges.insert( it, id ); }
    iterator erase  ( const iterator &it                       ) { return edges.erase ( it     ); }
    iterator erase  ( const iterator &it1, const iterator &it2 ) { return edges.erase ( it1,it2); }
    
    bool empty() {return edges.empty();}
    
    size_t size() const { return edges.size(); }
    size_t &operator[](size_t i) { return edges[i]; }
    const size_t &operator[](size_t i) const { return edges[i]; }
  };
}
