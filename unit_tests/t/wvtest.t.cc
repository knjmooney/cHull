#include <algorithm>
#include <cmath>
#include <iostream>

#include "wvtest.h"
#include "../src/point.hpp"
#include "../src/geometry.hpp"
#include "../src/face.hpp"
#include "../src/triangle.hpp"
#include "../src/edge.hpp"
#include "../src/unorderedEdge.hpp"
#include "../src/orderedEdge.hpp"
#include "../src/convexHull2D.hpp"
#include "../src/convexHull3D.hpp"
#include "../src/tile.hpp"
#include "../src/boundingBox.hpp"

#define EPS 0.00001f
#define WVPASSNEAR(a,b) WVPASS ( fabs(a-b) < fabs(a)*EPS + EPS )

using std::cout;
using std::endl;

// Prototype helper functions for testing
void projectToTile ( CompGeom::BoundingBox &B, const CompGeom::Geometry &geom );


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

WVTEST_MAIN("Point Class") {
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


WVTEST_MAIN("Geometry Class") {
  CompGeom::Geometry G { {0,0,0}, {1,1,0}, {1,0,0}, {1,1,1} };
  WVPASS ( G.getDim() == 3 );
  WVPASS ( G.size() == 4 );
  G.addPoint ( { 2,2,2 } );
  WVPASS ( G.size() == 5 );

  // Test points of incorrect dimension can't be added
  bool failed = false;
  try { G.addPoint ( { 1,1 } ); } catch(const std::exception&) { failed = true; }
  WVPASS ( failed ); 
  failed = false;
  try { G.addPoint ( { 1,1,1,1 } ); } catch(const std::exception&) { failed = true; }
  WVPASS ( failed );
}

WVTEST_MAIN("Face Class") {
  // Check constructor
  CompGeom::Point p1{{0,0,0}}, p2{{1,1,0}}, p3{{1,0,0}};  
  std::vector<CompGeom::Point> verts = {p1,p2,p3};
  CompGeom::Face f( verts ); 
  WVPASS ( f[0] == p1 );
  WVPASS ( f[1] == p2 );
  WVPASS ( f[2] == p3 );

  // Check norm and centre of mass are calculated correctly
  CompGeom::Point norm { {0,0,-1} }, com { {2.0/3.0, 1.0/3.0, 0} };
  WVPASS ( norm == f.normal() );
  WVPASS ( com  == f.com()    );

  // Check isVisible function
  CompGeom::Point q1{{1,1,1}}, q2{{1,3,-1}};
  WVPASS (  f.isVisible(q2) );
  WVPASS ( !f.isVisible(q1) );
}

WVTEST_MAIN("Triangle Class") {
  CompGeom::Geometry G { {0,0,0}, {1,1,0}, {1,0,0}, {1,1,1} };
  CompGeom::Triangle T ( 0,1,2,G );
  WVPASS ( T[0] == 0 );
  WVPASS ( T[1] == 1 );
  WVPASS ( T[2] == 2 );

  CompGeom::Point norm { {0,0,-1} }, com { {2.0/3.0, 1.0/3.0, 0} };
  WVPASS ( norm == T.normal() );
  WVPASS ( com  == T.com()    );

  // Check isVisible function
  CompGeom::Point q1{{1,1,1}}, q2{{1,3,-1}};
  WVPASS (  T.isVisible(q2) );
  WVPASS ( !T.isVisible(q1) );

  T.invert();
  WVPASS (  T.isVisible(q1) );
  WVPASS ( !T.isVisible(q2) );
  WVPASS ( T[0] == 0 );
  WVPASS ( T[1] == 2 );
  WVPASS ( T[2] == 1 );  
}

WVTEST_MAIN("Edge Struct") {
  CompGeom::UnorderedEdge e0(0,1), e1(1,0), e2(0,2);
  std::vector <CompGeom::UnorderedEdge> v = {e0,e1,e2}; 
  WVPASS ( v.size() == 3 );
  auto it = std::unique ( v.begin(), v.end() );
  WVPASS ( std::distance(v.begin(),it ) == 2 ); 
}

WVTEST_MAIN("Gift Wrap") {
  CompGeom::Geometry geom { {0,0}, {0,-1}, {1,0}, {-1,0}, {0,1} };
  std::vector < size_t > result = giftWrap(geom);
  result.pop_back(); 		// remove repeated index
  std::sort(result.begin(),result.end());
  WVPASS ( result == std::vector<size_t> ( { 1,2,3,4} ) );
}

WVTEST_MAIN("Graham Scan") {
  CompGeom::Geometry geom { {0,0}, {0,-1}, {1,0}, {-1,0}, {0,1} };
  std::vector < size_t > result = grahamScan(geom);
  result.pop_back(); 		// remove repeated index
  std::sort(result.begin(),result.end());
  WVPASS ( result == std::vector<size_t> ( { 1,2,3,4} ) );
}

// NEED TO REVISIT!!!
WVTEST_MAIN("Compare 2D algorithms") {
  CompGeom::Geometry geom{2};
  geom.addRandom(100000);	// This fails for >200,000

  std::vector < size_t > result1 = giftWrap  (geom);
  std::vector < size_t > result2 = grahamScan(geom);

  result1.pop_back(); 		// remove repeated index
  std::sort(result1.begin(),result1.end());
  result2.pop_back(); 		// remove repeated index
  std::sort(result2.begin(),result2.end());

  WVPASS ( result1 == result2 );
}


WVTEST_MAIN("3D Insertion Method") {
  CompGeom::Geometry geom { {0,0,0}, {0,-1,0}, {1,0,0}, {0,0,1}, {-1,0,0},
                            {0,1,0}, {0,0,-1} };
  auto tris = insertion3D(geom);
  std::vector<size_t> result;
  for ( auto&& t : tris ) {
    result.push_back(t[0]);
    result.push_back(t[1]);
    result.push_back(t[2]);
  }

  std::sort(result.begin(),result.end());
  auto it = std::unique(result.begin(),result.end());
  result.resize(std::distance(result.begin(),it));
  WVPASS ( result == std::vector<size_t> ( { 1,2,3,4,5,6 } ) );
}

WVTEST_MAIN("Tile") {
  CompGeom::Tile T( 32, 32 );

  T.set(0,0,1.1);
  WVPASSNEAR(T.get(0,0),1.1);
  WVPASS ( T.width() == 32 );
  WVPASS ( T.height() == 32 );
  

  bool failure = false;
  try {
    T.set(33,1,0.9);
  } catch ( std::out_of_range ) {
    failure = true;
  }
  WVPASS ( failure );

  T.set ( 31,31,0.9 );
  WVPASSNEAR ( T.get(31,31), 0.9 );

  CompGeom::Tile T_off( 32, 10 );
  T_off.set ( 31,9,0.9 );
  WVPASSNEAR ( T_off.get(31,9), 0.9 );
}

WVTEST_MAIN("Bounding Box") {

  CompGeom::Geometry geom { {0,0,0}, {0,-1,0}, {1,0,0}, {0,0,1}, {-1,0,0},
                            {0,1,0}, {0,0,-1} };
  CompGeom::BoundingBox B(32,32);
  CompGeom::Tile *T = B.up();

  projectToTile(B,geom);
  
  cout << T->get(15,15) << endl;
  WVPASSNEAR(T->get(15,15), 0 );
}
 
WVTEST_MAIN("gHull Serial") {
  CompGeom::Geometry geom { {0,0,0}, {0,-1,0}, {1,0,0}, {0,0,1}, {-1,0,0},
                            {0,1,0}, {0,0,-1} };

  auto tris = gHullSerial ( geom );  


  std::vector<size_t> result;
  for ( auto&& t : tris ) {
    cout << t[0] << endl;
    result.push_back(t[0]);
    result.push_back(t[1]);
    result.push_back(t[2]);
  }
  std::sort(result.begin(),result.end());
  auto it = std::unique(result.begin(),result.end());
  result.resize(std::distance(result.begin(),it));
  // WVPASS ( result == std::vector<size_t> ( { 1,2,3,4,5,6 } ) );
}

