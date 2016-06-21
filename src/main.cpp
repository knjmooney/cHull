/******************************************************
 * Name    : main.cpp
 * Author  : Kevin Mooney
 * Created : 13/06/16
 * Updated : 20/06/16
 *
 * Description:
 *
 ******************************************************/

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

#include "geometry.hpp"
#include "errorMessages.hpp"
#include "point_operations.hpp"

using namespace std;


// Gift wrap algorithm, this is much messier than I expected
vector< CompGeom::Point > giftWrap(const CompGeom::Geometry &geom) {
  if ( geom.getDim() != 2 ) {    
    errorM("Can only gift wrap 2D geometries\n");
  }  
  if ( geom.size() < 2 ) {
    errorM("Need more than 2 points to gift wrap\n");
  }
 
  vector < CompGeom::Point > cHull;

  // Find leftmost point and push it onto the hull
  auto first_on_hull_it = findMinX ( geom );
  auto curr_on_hull_it  = first_on_hull_it;
  CompGeom::Point dir_of_gift_wrap ( { 0,1 } );
  CompGeom::Point point_on_hull ( *curr_on_hull_it );
  cHull.push_back ( point_on_hull );

  // Assume there is at least more than one point
  do { 
    auto max_it = curr_on_hull_it;
    double max_angle = -numeric_limits<double>::max();	
    CompGeom::Point dir_to_point = dir_of_gift_wrap;

    // cout << "\n" << dir_of_gift_wrap << "\t" << *curr_on_hull_it << "\n";

    // Loop over all points and calculate the angle
    // made with the gift wrap
    for ( auto it = geom.begin(); it != geom.end(); it++ ) {
      if ( curr_on_hull_it != it ) {
	dir_to_point = *it-*curr_on_hull_it;
	double angle = cosv2 ( dir_of_gift_wrap, dir_to_point ) ;
	// cout << *it << "\t" << angle << endl;
	if ( angle > max_angle ) { 
	  max_angle = angle;
	  max_it = it;
	}
      }
    }
    // cout << *max_it << "\t" << *curr_on_hull_it << endl; 
    dir_of_gift_wrap = *max_it - *curr_on_hull_it;;
    curr_on_hull_it = max_it;
    cHull.push_back ( *curr_on_hull_it );
    
  // } while (0);
  } while ( first_on_hull_it != curr_on_hull_it );
  return cHull;
}

int main() {
  CompGeom::Geometry geom{2};
  
  geom.addRandom(5000);
  geom.print();

  vector < CompGeom::Point > hull = giftWrap(geom);

  cout << "\n\n";
  for ( auto i : hull ) {
    cout << i << endl;
  }

  // CompGeom::Point d({0.025,-0.263}),v({-0.4400, -0.3400});

  // cout << innerProduct(v,d) << endl;
  // cout << norm(d) << endl;

  // cout << cosv2(v,d) << endl;

  // ClockwiseAngle(p1,p2);
  // cout << -p2 << endl;
  // cout << p2 << endl;

  return EXIT_SUCCESS;
}
