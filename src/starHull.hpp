/******************************************************
 * Name    : starHull.hpp
 * Author  : Kevin Mooney
 * Created : 25/08/16
 * Updated : 
 *
 * Description:
 *
 * ToDo:
 ******************************************************/

#include <set>
#include <string>
#include <vector>

#include "star.hpp"
#include "triangle.hpp"

namespace CompGeom {
  class StarHull {

  private:
    std::vector < Star > current;
    std::vector < std::vector < Star > > old; 
    const Geometry geom;

  public:
    StarHull ( const Geometry & geom ) : current{}, old{}, geom{geom} {}

    template < typename inputIt >
    void update ( const inputIt start, const inputIt end ) {
      if ( !current.empty() ) old.push_back(current);
      current.assign(start,end);
    }

    void print ( const std::string &file_name );
  };


  inline void StarHull::print ( const std::string & file_name ) {
    std::ofstream file ( file_name );
    file << "META DATA" << std::endl;
    file << "TYPE\tCONVEX HULL" << std::endl;
    file << "Number of Points " << geom.size() << std::endl;
    file << "Number of Timesteps " << old.size() << std::endl;
    file << std::endl << std::endl;
    
    for ( size_t i=0; i<geom.size(); i++ ) {
      file << "POINT " << i << " " << geom[i] << std::endl;
    }
    file << std::endl << std::endl;
    
    size_t count = 0;
    for ( size_t si=0; si<old.size(); si++ ) {
      const auto & star_list = old[si];
      std::set<Triangle> tri_list;
      for ( auto star : star_list ) {
	tri_list.insert ( Triangle( star.id, star.back(), star.front(),geom ) );
	for ( size_t j = 0; j<star.size()-1; j++ ) {
	  tri_list.insert ( Triangle( star.id, star[j], star[j+1],geom ) );
	}
      }
      
      
      file << "#################################" << std::endl;
      file << "Timestep " << count++ << std::endl;
      file << "Number of Triangles " << tri_list.size() << std::endl;
      size_t count = 0;
      for ( const auto & tri : tri_list )  {
	file << "Triangle " << count++ << " " << tri[0] << " " << tri[1] << " " << tri[2] << std::endl;;
      }
      file << std::endl << std::endl;
    
      file << "Number of Stars " << star_list.size() << std::endl;
      for ( size_t i=0; i<star_list.size(); i++ ) {
	const auto & star = star_list[i];
	file << "Star " << i << " " << "alive " << star.id << " ";
	for ( const auto &edge : star ) file << edge << " ";
	file << std::endl;
      }
      file << std::endl << std::endl;
    }
  }
}


