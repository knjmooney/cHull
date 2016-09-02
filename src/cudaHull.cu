/***************************************************
 * Name    : cudaHull.cu
 * Author  : Kevin Mooney
 * Created : 01/07/16
 * Updated : 04/07/16
 *
 * Description:
 *   Monotone chains implementation using thrust
 * 
 * TODO:
 *   IMPLEMENT TEMPLATES in geometry, point and here!
 *
 * NOTES:
 *   This code could drastically improve in readabilty 
 *   if a class was used to store the chains
 ******************************************************/

#include <thrust/adjacent_difference.h>
#include <thrust/copy.h>
#include <thrust/iterator/constant_iterator.h>
#include <thrust/device_vector.h>
#include <thrust/host_vector.h>
#include <thrust/remove.h>
#include <thrust/sort.h>



// Jose doesn't use vector at all...
#include <iostream>
#include <vector>

#include "cudaHull.hpp"
#include "geometry.hpp"

typedef thrust::tuple<int,float,float,bool> id_coord_rotation_tuple; 
typedef thrust::tuple<int,float,float> id_coord_tuple; 
typedef thrust::tuple<float,float> coord_tuple; 

// functor for comparing x-value of float2s
struct compareX : public thrust::binary_function<id_coord_tuple,id_coord_tuple,bool> { 
  __host__ __device__
  bool operator()(const id_coord_tuple &a, const id_coord_tuple &b) {
    return a.get<1>() < b.get<1>();
  }
};

// Returns whether the cross product between two vectors is positive
struct rotation : public thrust::binary_function<coord_tuple,coord_tuple,bool> { 
  __host__ __device__
  bool operator() ( const coord_tuple &a, const coord_tuple &b) {
    return ( a.get<1>()*b.get<0>() - a.get<0>()*b.get<1>() ) > 0;
  }
};

// Upper Chain
// Determines if point currently makes a convex angle with neighbouring points
struct upwardFacing : public thrust::unary_function<id_coord_rotation_tuple,bool> { 
  __host__ __device__
  bool operator() ( const id_coord_rotation_tuple &a) {
    return !a.get<3>();
  }
};

// Lower Chain
// Determines if point currently makes a convex angle with neighbouring points
struct downwardFacing : public thrust::unary_function<id_coord_rotation_tuple,bool> { 
  __host__ __device__
  bool operator() ( const id_coord_rotation_tuple &a) {
    return a.get<3>();
    // return true;
  }
};

std::vector< size_t > cudaHull ( const CompGeom::Geometry &geom ) {

  // Transfer geometry to host vector
  thrust::host_vector<float> h_xvec(geom.size());
  thrust::host_vector<float> h_yvec(geom.size());
  for ( size_t i=0; i<geom.size(); i++ ) {
    h_xvec[i] = geom[i][0];
    h_yvec[i] = geom[i][1];
  }

  // transfer data to the device                                                                  
  thrust::device_vector<float> d_xvec = h_xvec;
  thrust::device_vector<float> d_yvec = h_yvec;

  // Fill an array of IDS on device and sort them along with the points
  // by their x-coordinate
  thrust::device_vector<int> ids(d_xvec.size());
  thrust::sequence(ids.begin(),ids.end());

  // Zip IDS and coordinates
  // auto zip_start = make_zip_iterator(make_tuple(ids.begin(), 
  // 						d_xvec.begin(),
  // 						d_yvec.begin()));
  // auto zip_end   = make_zip_iterator(make_tuple(ids.end(), 
  // 						d_xvec.end(),
  // 						d_yvec.end()));
  //  Sort by x-coordinate
  // thrust::sort(zip_start, zip_end, compareX());
  // This sped it up significantly
  thrust::sort_by_key(d_xvec.begin(),
  		      d_xvec.end(),
  		      make_zip_iterator(make_tuple(ids.begin(), d_yvec.begin()) ) );

  // Copy ids and vectors into lower chain
  thrust::device_vector<int>   lower_ids(ids.begin()   ,ids.end()   );
  thrust::device_vector<float> lower_x  (d_xvec.begin(),d_xvec.end());
  thrust::device_vector<float> lower_y  (d_yvec.begin(),d_yvec.end());

  // Calculate the vectors between each point
  thrust::device_vector<float> vdiff_x(ids.size());
  thrust::device_vector<float> vdiff_y(ids.size());
  thrust::adjacent_difference(d_xvec.begin(),d_xvec.end(),vdiff_x.begin());
  thrust::adjacent_difference(d_yvec.begin(),d_yvec.end(),vdiff_y.begin());

  // Determine if the angle between consecutive vectors is convex up
  // End vectors are always true
  thrust::device_vector<bool> upper_convex(ids.size());
  thrust::fill(upper_convex.begin(),upper_convex.end(),true);
  auto diff_start = make_zip_iterator ( make_tuple ( vdiff_x.begin(), vdiff_y.begin() ) );
  auto diff_end   = make_zip_iterator ( make_tuple ( vdiff_x.end()  , vdiff_y.end()   ) );  
  thrust::transform ( diff_start+2, diff_end, diff_start+1, upper_convex.begin()+1, rotation() );
  
  // Remove downward facing points
  bool * upper_convex_ptr = thrust::raw_pointer_cast(upper_convex.data());
  auto four_tuple_start = make_zip_iterator ( make_tuple ( ids.begin(),
							   d_xvec.begin(),
							   d_yvec.begin(),
							   upper_convex.begin()));
  auto four_tuple_end   = make_zip_iterator ( make_tuple ( ids.end(),
							   d_xvec.end(),
							   d_yvec.end(),
							   upper_convex.end()));
  auto new_end = thrust::remove_if ( four_tuple_start, four_tuple_end, upwardFacing() ); 

  // auto a = new_end.get_iterator_tuple();

  int old_N = 0;
  int N     = thrust::distance (four_tuple_start, new_end );
  
  while ( old_N != N ) {
    thrust::adjacent_difference(d_xvec.begin(),d_xvec.begin() + N ,vdiff_x.begin());
    thrust::adjacent_difference(d_yvec.begin(),d_yvec.begin() + N ,vdiff_y.begin());
    
    thrust::transform ( diff_start+2, diff_start + N , diff_start+1, upper_convex.begin()+1, rotation() );
    
    new_end = thrust::remove_if ( four_tuple_start, four_tuple_start + N, upwardFacing() ); 
    
    old_N = N;
    N = thrust::distance (four_tuple_start, new_end );
  }

  four_tuple_start = make_zip_iterator ( make_tuple ( lower_ids.begin(),
						      lower_x.begin(),
						      lower_y.begin(),
						      upper_convex.begin()));
  four_tuple_end   = make_zip_iterator ( make_tuple ( lower_ids.end(),
						      lower_x.end(),
						      lower_y.end(),
						      upper_convex.end()));

  // LOWER CHAIN
  thrust::adjacent_difference(lower_x.begin(),lower_x.end(),vdiff_x.begin());
  thrust::adjacent_difference(lower_y.begin(),lower_y.end(),vdiff_y.begin());
  thrust::fill(upper_convex.begin(),upper_convex.end(),false);
  thrust::transform ( diff_start+2, diff_end, diff_start+1, upper_convex.begin()+1, rotation() );
  new_end = thrust::remove_if ( four_tuple_start, four_tuple_end, downwardFacing() ); 

  int upper_N = N; 		// Need a better naming convention
  old_N = 0;
  N = thrust::distance (four_tuple_start, new_end );
  while ( old_N != N ) {
    thrust::adjacent_difference(lower_x.begin(),lower_x.begin() + N ,vdiff_x.begin());
    thrust::adjacent_difference(lower_y.begin(),lower_y.begin() + N ,vdiff_y.begin());

    thrust::transform ( diff_start+2, diff_start + N , diff_start+1, upper_convex.begin()+1, rotation() );

    new_end = thrust::remove_if ( four_tuple_start, four_tuple_start + N, downwardFacing() );

    old_N = N;
    N = thrust::distance (four_tuple_start, new_end );
  }

  // Copy back to host and return
  std::vector< size_t > result (N + upper_N - 1);
  thrust::copy(ids.begin(), ids.begin()+upper_N, result.begin());
  thrust::copy(lower_ids.rend()-N + 1, lower_ids.rend(), result.begin()+upper_N);

  return result;
}
