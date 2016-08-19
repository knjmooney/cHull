/******************************************************
 * Name    : geometryHelper.hpp
 * Author  : Kevin Mooney
 * Created : 25/07/16
 * Updated :
 *
 * Description:
 *
 * NOTES:
 ******************************************************/

#pragma once

#include <vector>

// Finds the minimum and maximum coordinates in all dimensions
std::vector < float > findExtremes2 ( const CompGeom::Geometry &geom );

// Deprecated
// Finds the minimum and maximum coordinates in all dimensions
void findExtremes ( const CompGeom::Geometry &geom, float min[3], float max[3] );
