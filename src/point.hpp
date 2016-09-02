/******************************************************
 * Name    : point.hpp
 * Author  : Kevin Mooney
 * Created : 13/06/16
 * Updated : 15/06/16
 *
 * Description:
 *
 * ToDo:
 *    - Possible template coord type
 *    - Understand why the operator<< cannot be a member
 *      function
 *    - Allow for auto range based loops
 *    - Implement const types
 *    - Implement range based comparison
 *    - Should overloaded operators check size()?
 ******************************************************/

#pragma once

#include <algorithm>
#include <cmath>
#include <ostream>
#include <vector>

namespace CompGeom {
  
  class Point {
    
  private:
    std::vector<float> coord;
    
  public:
    explicit Point ( size_t _dim ) : coord { std::vector<float>(_dim,0.0) } {} 
    Point ( std::vector<float> _coord ) : coord{_coord} {} 
    ~Point() {}

    Point ( std::initializer_list<float> _p ) : coord{_p} {}    
    Point operator= ( std::initializer_list<float> _p ) { return coord = _p; }

    typedef typename std::vector<float>::iterator iterator;
    typedef typename std::vector<float>::const_iterator const_iterator;
    iterator        begin()        { return coord.begin(); }
    iterator        end()          { return coord.end();   }
    const_iterator  begin() const  { return coord.begin(); }
    const_iterator  end()   const  { return coord.end();   }

    size_t size() const  { return coord.size(); }
    float length() const;

    // Operator overloading
    float operator   [](int i) const  {return coord[i];}
    float &operator  [](int i)        {return coord[i];}
    friend std::ostream& operator<<(std::ostream& os, const Point &p);
    friend Point operator- ( const Point &p );
    friend Point operator- ( const Point &p1, const Point &p2 );
    friend Point operator+ ( const Point &p1, const Point &p2 );
    friend bool  operator==( const Point &p1, const Point &p2 );
    friend bool  operator!=( const Point &p1, const Point &p2 );
    friend float operator* ( const Point &p1, const Point &p2 );
  };

  inline float Point::length() const { 
    float sum = 0.0;
    for ( auto i : coord ) {
      sum += i*i;
    }
    return sqrt ( sum );
  }

  // This is defined as a friend, not sure why it didn't work when it was a memeber function...
  // Should most likely exclude pretty formatting, so it can be used for plotting later
  // Could implement a setw
  inline std::ostream& operator<<(std::ostream& os, const Point &p)
  {
    auto it = p.coord.begin();
    for ( ; it+1 != p.coord.end(); it++ ) {
      os << *it << ' ';
    }
    os << *it; 
    return os;
  }
  
  inline Point operator- ( const Point &p1, const Point &p2 ) {
    Point p_diff(p1);
    std::transform ( p_diff.begin(), p_diff.end(), p2.begin(),
		     p_diff.begin(), std::minus<float>() );
    return p_diff;
  }
  
  inline Point operator- ( const Point &p) {  
    Point p_inv(p);
    for ( size_t i=0; i<p.size(); i++ ) {
      p_inv[i] = -p[i];
    }
    return p_inv;
  }

  inline Point operator+ ( const Point &p1, const Point &p2 ) {
    Point p_diff(p1);
    std::transform ( p_diff.begin(), p_diff.end(), p2.begin(),
		     p_diff.begin(), std::plus<float>() );
    return p_diff;
  }


  inline bool operator==( const Point &p1, const Point &p2 ) {
    if ( p1.size() != p2.size() ) return false;
    else return std::equal ( p1.begin(), p1.end(), p2.begin() );
  }

  inline bool operator!=( const Point &p1, const Point &p2 ) {
    return ! (p1 == p2);
  }

  // Should I check p1.size() == p2.size()?
  inline float operator* ( const Point &p1, const Point &p2 ) {
    float sum = 0;
    for ( size_t i=0; i<p1.size(); i++ ) {
      sum += p1[i]*p2[i];
    }
    return sum;
  }
}


