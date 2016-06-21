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

#include <cmath>
#include <ostream>
#include <vector>

namespace CompGeom {
  
  class Point {
   
  private:
    std::vector<double> coord;

  public:
    Point ( std::vector<double> _coord) : coord{_coord} {} 
    ~Point () {}

    typedef typename std::vector<double>::iterator iterator;
    typedef typename std::vector<double>::const_iterator const_iterator;
    iterator        begin()        { return coord.begin(); }
    iterator        end()          { return coord.end();   }
    const_iterator  begin() const  { return coord.begin(); }
    const_iterator  end()   const  { return coord.end();   }

    size_t size() const  { return coord.size(); }
    double length() const;

    // Operator overloading
    double operator   [](int i) const  {return coord[i];}
    double &operator  [](int i)        {return coord[i];}
    friend std::ostream& operator<<(std::ostream& os, const Point &p);
    friend Point operator- ( const Point &p );
    friend Point operator- ( const Point &p1, const Point &p2 );
    friend bool  operator==( const Point &p1, const Point &p2 );
    friend bool  operator!=( const Point &p1, const Point &p2 );
    friend double operator* ( const Point &p1, const Point &p2 );
  };

  double Point::length() const { 
    double sum = 0.0;
    for ( auto i : coord ) {
      sum += i*i;
    }
    return sqrt ( sum );
  }

  // This is defined as a friend, not sure why it didn't work when it was a memeber function...
  // Should most likely exclude pretty formatting, so it can be used for plotting later
  // Could implement a setw
  std::ostream& operator<<(std::ostream& os, const Point &p)
  {
    auto it = p.coord.begin();
    for ( ; it+1 != p.coord.end(); it++ ) {
      os << *it << ' ';
    }
    os << *it; 
    return os;
  }
  
  Point operator- ( const Point &p1, const Point &p2 ) {
    Point p_diff(p1);
    for ( size_t i=0; i<p1.size(); i++ ) {
      p_diff[i] -= p2[i];
    }
    return p_diff;
  }
  
  Point operator- ( const Point &p) {  
    Point p_inv(p);
    for ( size_t i=0; i<p.size(); i++ ) {
      p_inv[i] = -p[i];
    }
    return p_inv;
  }

  bool operator==( const Point &p1, const Point &p2 ) {
    for ( size_t i = 0; i<p1.size(); i++ ) {
      if ( p1[i] != p2[i] ) return false;
    } 
    return true;
  }

  bool operator!=( const Point &p1, const Point &p2 ) {
    return ! (p1 == p2);
  }

  // Should I check p1.size() == p2.size()?
  double operator* ( const Point &p1, const Point &p2 ) {
    double sum = 0;
    for ( size_t i=0; i<p1.size(); i++ ) {
      sum += p1[i]*p2[i];
    }
    return sum;
  }
}
