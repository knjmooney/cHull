/******************************************************
 * Name    : insertion3D.hpp
 * Author  : Kevin Mooney
 * Created : 25/07/16
 * Updated :
 *
 * Description:
 *
 * NOTES:
 ******************************************************/

#pragma once

#include <string>

#include "geometry.hpp"

// As each triangle is oriented with some normal, all edges are entered
// such that the vertices are ordered anti-clockwise when viewing triangle from 
// the normal
std::vector < std::vector < size_t > > insertion3D ( const CompGeom::Geometry &geom ); 
std::vector < std::vector < size_t > > insertion3D ( const CompGeom::Geometry &geom, const std::string &filename ); 


