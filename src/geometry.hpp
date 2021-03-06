/******************************************************
 * Name    : geometry.hpp
 * Author  : Kevin Mooney
 * Created : 13/06/16
 * Updated : 19/07/16
 *
 * Description:
 *   Vector wrap around class for storing a list of coordinates
 *
 * ToDo:
 *   - rethink hardcoded seed, possible have gen as 
 *     member of class
 *   - Make addRandom work for generic box sizes
 *   - Add points from file
 *   - Add method for checking integrity of geometry 
 *   - Template the dimension of the geometry
 *
 * Notes:
 *   - Normal distribution gives uninitialised warnings
 *     for old versions of gcc
 *   - All code has been written in header file for 
 *     convenience with later templating
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
    size_t const dim;		// The dimension of the geometry, could be templated
    std::vector<Point> coords;

  public:
    Geometry(const size_t &_dim) : dim{_dim} {}
    Geometry(const std::vector<Point> &_coords) : dim{_coords[0].size()}, coords{_coords} {}
    Geometry(std::initializer_list<Point> P ) : dim{(*P.begin()).size()}, coords{P} {} // ugly

    typedef typename std::vector<Point>::iterator iterator;
    typedef typename std::vector<Point>::const_iterator const_iterator;
    iterator       begin()        { return coords.begin(); }
    iterator       end()          { return coords.end();   }
    const_iterator begin() const  { return coords.begin(); }
    const_iterator end()   const  { return coords.end();   }

    size_t size() const { return coords.size(); }
    size_t getDim() const { return dim; }

    Point operator   [](int i) const  {return coords[i];}
    Point &operator  [](int i)        {return coords[i];}

    // Adds N random points, in a box around the origin
    void addRandom(int N);
    void addPoint (std::initializer_list<float> P);
    void translate(const std::vector<float> &shift);

    // Prints the geometry to stdout
    void print();

    // Pretty prints the geometry with some meta data 
    // and the triangles that construct the convex hull
    void printHull(const std::string &file_name, const std::vector<size_t> &cHull) const;
    void print3DGeom(const std::string &file_name, int steps) const;
    void append3DHull(const std::string &file_name, const std::vector < std::vector<size_t> > &cHull, int step) const;
  };

  // normal_dist gives uninitialised warning for old versions of gcc
  inline void Geometry::addRandom(int N) {    
    std::default_random_engine      gen(1432543);
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

  inline void Geometry::addPoint (std::initializer_list<float> P) { 
    if ( P.size() != dim ) errorM("Points need to be of same dimension as geometry"); 
      coords.push_back(P); 
  }
  
  // Prints the geometry to screen
  inline void Geometry::print() {
    for ( auto i : coords ) {
      std::cout << i << std::endl;
    }
  }

  // && is a move semantic, unsure if it's been properly implemented
  inline void Geometry::translate(const std::vector<float> &shift) {
    if ( shift.size() != dim ) {
      errorM("Shift vector must be of same dimension as geometry\n");
    }
    for ( auto&& p : coords ) {
      for ( size_t i=0; i<p.size(); i++ ) {
	p[i] += shift[i];
      }
    }
  }

  // Very messy, specialised for printing 2D geometries to be modelled in 3D
  inline void Geometry::printHull(const std::string &file_name, const std::vector<size_t> &cHull) const {
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

  inline void Geometry::print3DGeom(const std::string &file_name, int steps) const {
    std::ofstream file ( file_name );
    // file = std::cout;
    file << "META DATA \n";
    file << "TYPE " << "Convex Hull" << "\n";
    file << "Number of Points " << size() << "\n";
    file << "Number of Timesteps " << steps << "\n";
    // file << "DIMENSION\t\t"  << dim << "\n";
    // file << "GEN\t\t RANDOM (NORMAL DIST)\n";
    // file << "\n\nSTART COORDINATES \n";
    int c=0;
    file << "\n";
    for ( auto i : coords ) file << "POINT " << c++ << " " << i << std::endl;
    // file << "\n\nSTART HULL \n";
    // for ( auto i : cHull) { for ( auto j : i ) file << j << " "; file << std::endl; }
  }

  inline void Geometry::append3DHull
  (const std::string &file_name, const std::vector < std::vector<size_t> > &cHull, int step) const {
    std::ofstream file ( file_name , std::ofstream::app);
    file << "\n\nTimestep " << step << "\n";
    file << "Number of Triangles " << cHull.size() << "\n";
    file << "START HULL\n";
    for ( auto i : cHull) { for ( auto j : i ) file << j << " "; file << std::endl; }
  }
}

