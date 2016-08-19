/******************************************************
 * Name    : boundingBox.hpp
 * Author  : Kevin Mooney
 * Created : 25/07/16
 * Updated : 03/08/16
 *
 * Description:
 *
 * NOTES:
 ******************************************************/

#include "boundingBox.hpp"
#include "geometryHelper.hpp"

using namespace CompGeom;
using namespace Direction;

BoundingBox::BoundingBox ( const size_t &w ) 
  : _T{std::vector<Tile>(6,Tile(w))}, _length{w} {}


											 
