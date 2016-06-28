/******************************************************
 * Name    : geometry.hpp
 * Author  : Kevin Mooney
 * Created : 13/06/16
 * Updated : 15/06/16
 *
 * Description:
 *
 * ToDo:
 *   - rethink hardcoded seed, possible have gen as 
 *   - member of class
 *   - Make addRandom work for generic box sizes
 *   - Understand const methods
 *   - Add custom points
 *   - Add points from file
 ******************************************************/

#pragma once

#include <iostream>
#include <random>
#include <vector>

#include "point.hpp"
#include "errorMessages.hpp"

namespace CompGeom {

  class Geometry {

  private:
    // std::vector<int> const dims;
    size_t const dim;
    std::vector<Point> coords;

    // Maybe useful, not necessarily....
    typedef std::uniform_real_distribution<double> urdist;
  public:
    Geometry(const size_t &_dim) : dim{_dim} {}
    Geometry(const std::vector<Point> &_coords) : dim{_coords[0].size()}, coords{_coords} {}
    
    // Const method, unsure if this is necessary
    size_t getDim() const { return dim; }

    // typedef iterators, might need to improve these to work with auto
    // Possibly need const types here too
    typedef typename std::vector<Point>::iterator iterator;
    typedef typename std::vector<Point>::const_iterator const_iterator;
    iterator       begin()        { return coords.begin(); }
    iterator       end()          { return coords.end();   }
    const_iterator begin() const  { return coords.begin(); }
    const_iterator end()   const  { return coords.end();   }

    size_t size() const { return coords.size(); }

    Point operator   [](int i) const  {return coords[i];}
    Point &operator  [](int i)        {return coords[i];}
    
    // Adds N random points, in a box around the origin
    // Bad naming convention....
    // normal_dist gives warning
    void addRandom(int N) {

      std::default_random_engine gen(1432543);
      // urdist dist(-1.0,1.0);
      std::normal_distribution<double> dist(0.0,1.0);
      // std::cauchy_distribution<double> dist(5.0,1.0);

      for ( int i=0; i<N; i++ ) {
	std::vector<double> point;
	for ( size_t j=0; j<dim; j++ ) {
	  point.push_back( dist(gen) );
	}

	Point p(point);
	coords.push_back(p);	// Could change this to a move... not important
      }
    }

    void print() {
      for ( auto i : coords ) {
      	std::cout << i << std::endl;
      }
    }

    void translate(const std::vector<double> &shift) {
      if ( shift.size() != dim ) {
	errorM("Shift vector must be of same dimension as geometry\n");
      }
      for ( auto&& p : coords ) {
	for ( size_t i=0; i<p.size(); i++ ) {
	  p[i] += shift[i];
	}
      }
    }
  };
}
