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

#include <fstream>
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
    typedef std::uniform_real_distribution<float> urdist;
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
    void addRandom(int N);
    void translate(const std::vector<float> &shift);

    void print();
    void printHull(const std::string &file_name, const std::vector<size_t> &cHull);
  };

  void Geometry::addRandom(int N) {    
    std::default_random_engine gen(1432543);
    // urdist dist(-1.0,1.0);
    std::normal_distribution<float> dist(0.0,1.0);
    // std::cauchy_distribution<float> dist(5.0,1.0);
    
    for ( int i=0; i<N; i++ ) {
      std::vector<float> point;
      for ( size_t j=0; j<dim; j++ ) {
	point.push_back( dist(gen) );
      }
      
      Point p(point);
      coords.push_back(p);	// Could change this to a move... not important
    }
  }


  void Geometry::print() {
    for ( auto i : coords ) {
      std::cout << i << std::endl;
    }
  }

  void Geometry::translate(const std::vector<float> &shift) {
    if ( shift.size() != dim ) {
      errorM("Shift vector must be of same dimension as geometry\n");
    }
    for ( auto&& p : coords ) {
      for ( size_t i=0; i<p.size(); i++ ) {
	p[i] += shift[i];
      }
    }
  }

  void Geometry::printHull(const std::string &file_name, const std::vector<size_t> &cHull) {
    std::ofstream file ( file_name );
    file << "META DATA \n";
    file << "TYPE\t\t" << "Convex Hull" << "\n";
    file << "NP IN GEOM\t\t" << size() << "\n";
    file << "NP IN HULL\t\t" << cHull.size()-1 << "\n";
    file << "DIMENSION\t\t" << dim << "\n";
    file << "GEN\t\t RANDOM (NORMAL DIST)\n";
    file << "\n\nSTART COORDINATES \n";
    file << 0 << " " << 0 << " " << 0 << " " << 0 << "\n";
    for ( size_t i=0; i<size(); i++ ) file << i+1 << " " << coords[i] << " " << 0 << "\n";
    file << "\n\nSTART HULL \n";
    for ( size_t i=1; i<cHull.size(); i++ ) file << cHull[i-1]+1 << " " << cHull[i]+1 << " " << 0 <<  "\n";
    file << std::endl;
  }

}
