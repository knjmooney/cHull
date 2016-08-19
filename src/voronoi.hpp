/******************************************************
 * Name    : voronoi.hpp
 * Author  : Kevin Mooney
 * Created : 12/08/16
 * Updated :
 *
 * Description:
 *
 * NOTES:
 ******************************************************/

#pragma once

#include <vector>
#include <string>

typedef std::vector < std::vector < short > > VoronoiRow;
typedef std::vector < std::vector < std::vector < short > > > Voronoi;

// Stolen from Rosetta Code
// Creates a portable bitmap file with a Voronoi diagram
void makeVoronoiPBM( short * V, short * In, size_t w, size_t h, const std::string &filename );
void makeVoronoiPBM( const Voronoi &V, const std::string &filename );

