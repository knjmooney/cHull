/******************************************************
 * Name    : convexHull2D.hpp
 * Author  : Kevin Mooney
 * Created : 25/07/16
 * Updated : 
 *
 * Description:
 *
 * NOTES:
 *  Fixed Graham Scan Bug 25/7/16
 ******************************************************/

#pragma once

#include "geometry.hpp"
#include "point.hpp"

// Gift wrap algorithm
std::vector< size_t > giftWrap(const CompGeom::Geometry &geom);

// Graham Scan algorithm
std::vector< size_t > grahamScan(CompGeom::Geometry &geom);

