/******************************************************
 * Name    : convexHull.hpp
 * Author  : Kevin Mooney
 * Created : 20/07/16
 * Updated : 
 *
 * Description:
 *
 * ToDo:
 ******************************************************/

#include <string>
#include <vector>

#include "triangle.hpp"

namespace CompGeom {
  class ConvexHull3D {

  private:
    std::vector < Triangle > current;
    std::vector < std::vector < Triangle > > old; 
    const Geometry geom;

  public:
    ConvexHull3D ( const Geometry & geom ) : current{}, old{}, geom{geom} {}

    template < typename inputIt >
    void update ( const inputIt start, const inputIt end ) {
      if ( !current.empty() ) old.push_back(current);
      current.assign(start,end);
    }

    void print ( const std::string &file_name );
  };


  inline void ConvexHull3D::print ( const std::string & file_name ) {
    std::ofstream file ( file_name );

    file << "META DATA \n";
    file << "TYPE " << "Convex Hull" << "\n";
    file << "Number of Points " << geom.size() << "\n";
    file << "Number of Timesteps " << old.size() << "\n";

    int c=0;
    file << "\n";
    for ( auto i : geom ) file << "POINT " << c++ << " " << i << std::endl;

    size_t i;
    for ( i=0; i<old.size(); i++ ) {
      file << "\n\n";
      file << "Timestep " << i << "\n";
      file << "Number of Triangles " << old[i].size() << "\n";
      file << "START HULL\n";
      for ( auto tri : old[i] ) 
	file << tri[0] << " " << tri[1] << " " << tri[2] << "\n";
    }

    file << "\n\n";
    file << "Timestep " << i << "\n";
    file << "Number of Triangles " << current.size() << "\n";
    file << "START HULL\n";
    for ( auto tri : current ) 
      file << tri[0] << " " << tri[1] << " " << tri[2] << "\n";
  }
}
