/***************************************************
 * Name    : cudaHull.hpp
 * Author  : Kevin Mooney
 * Created : 01/07/16
 * Updated : 
 *
 * Description:
 *
 * TODO:
 *   - Should functions be declard in CompGeom too
 ******************************************************/

#pragma once

#include <vector>

#include "geometry.hpp"

std::vector< size_t > cudaHull ( const CompGeom::Geometry& );  

