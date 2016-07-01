#include <cmath>
#include <iostream>

#include "wvtest.h"
#include "../src/point.hpp"
#include "../src/geometry.hpp"

#define EPS 0.00001f
#define WVPASSNEAR(a,b) WVPASS ( fabs(a-b) < fabs(a)*EPS )

WVTEST_MAIN("wvtest tests")
{
    WVPASS(1);
    WVPASSEQ(1, 1);
    WVPASSNE(1, 2);
    WVPASSEQ(1, 1);
    WVPASSLT(1, 2);

    WVPASSEQ("hello", "hello");
    WVPASSNE("hello", "hello2");

    WVPASSEQ(std::string("hello"), std::string("hello"));
    WVPASSNE(std::string("hello"), std::string("hello2"));
}

WVTEST_MAIN("Point Class")
{
  std::vector<float> v = {3.2,2.1};
  CompGeom::Point p1 ( 2 ), p2 ( v );

  for ( size_t i=0; i<p2.size(); i++ ) {
    WVPASSNEAR(v[i],p2[i]);
  }

  p1[0] = 2.2;
  p1[1] = 1.1;

  p2 = p2 - p1;

  WVPASSNEAR(p2[0],1.0);
  WVPASSNEAR(p2[1],1.0);

  WVPASS ( p1 == p1 );
  WVPASS ( p1 != p2 );

  WVPASSNEAR(p1*p1,6.05);
}


WVTEST_MAIN("Geometry Class")
{
  CompGeom::Geometry p1 { 2 };
}
