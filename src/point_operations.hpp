/******************************************************
 * Name    : point_operations.hpp
 * Author  : Kevin Mooney
 * Created : 20/06/16
 * Updated : 
 *
 * Description:
 *
 * ToDo:
 ******************************************************/


// This is a std algorithm...
CompGeom::Geometry::const_iterator findMinX(const CompGeom::Geometry &geom) {
  auto it     = geom.begin();
  auto it_min = it;
  it++;
  for ( ; it != geom.end(); it++ ) {
    if ( (*it)[0] < (*it_min)[0] ) it_min = it;
  }
  return it_min;
}

// Caluclates the 2 norm of a vector
template < typename T >
inline double innerProduct ( const T &x, const T &y ) {
  //  if ( x.size() != y.size() )
  double sum = 0.0;
  for ( size_t i=0; i<x.size(); i++ ) {
    sum += x[i]*y[i];
  }
  return sum;
}

// Caluclates the 2 norm of a vector
template < typename T >
inline double norm ( const T &x ) {
  return sqrt ( innerProduct ( x, x ) );
}

// T needs [] overloaded and routines length() and size()
// Should get rid of length() dependence
template < typename T >
double innerAngle ( const T &x, const T &y ) {
  if ( x.size() != 2 && y.size() != 2 ) {
    std::cerr << "innerAngle: Can only calculate 2D points\n";
    return 0.0;
  }
  return acos(cosv2(x,y));
}


template < typename T >
double antiClockwiseAngle ( const T &x, const T &y ) {
  if ( x.size() != 2 || y.size() != 2 ) {
    errorM("Can only find angle for 2D points");
  }
  // Shamelessly stolen from Stack Overflow
  double dot = x[0]*y[0] + x[1]*y[1];
  double det = x[0]*y[1] - x[1]*y[0];
  return atan2(det, dot);
}

// Calculates the cosine of the angle between two vectors
// May need to rewrite this to be specific to a plane
template < typename T >
inline double cosv2 ( const T &x, const T &y ) {
  return innerProduct ( x, y ) / ( norm ( x ) * norm ( y ) );
}
