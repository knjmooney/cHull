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

#include <vector>

#include "workingSet.hpp"

namespace CompGeom {

  class Star {
  private:
    typedef std::vector<size_t> container;
    
  public:
    const size_t id;
    container edges;
    
    Star ( const size_t &id ) : id{id} {}

    typedef typename container::iterator iterator;
    typedef typename container::const_iterator const_iterator;
    iterator       begin()        { return edges.begin(); }
    iterator       end()          { return edges.end();   }
    const_iterator begin() const  { return edges.begin(); }
    const_iterator end()   const  { return edges.end();   }
    
    size_t front() const { return edges.front(); }
    size_t back()  const { return edges.back (); }

    void insert ( const iterator &it, const size_t &id ) { edges.insert( it, id ); }
    
    size_t size() const { return edges.size(); }
    size_t operator[](size_t i) { return edges[i]; }
  };
}
