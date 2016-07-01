/***************************************************
 * Name    : cudaHull.cu
 * Author  : Kevin Mooney
 * Created : 01/07/16
 * Updated : 
 *
 * Description:
 *
 * TODO:
 ******************************************************/

// #include <thrust/host_vector.h>
// #include <thrust/device_vector.h>
// #include <thrust/generate.h>
// #include <thrust/sort.h>
// #include <thrust/copy.h>

// Jose doesn't use vector at all...
#include <vector>

// #include "cudaHull.hpp"

#include "geometry.hpp"

std::vector< std::size_t > cudaHull ( const std::vector< std::size_t > &geom ) {
  return std::vector< std::size_t > (1);
}
