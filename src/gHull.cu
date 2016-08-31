/******************************************************
 * Name    : gHull.hup
 * Author  : Kevin Mooney
 * Created : 26/08/16
 * Updated :
 *
 * Description:
 *
 * NOTES:
 *   Class ideas were taken from cuda quadtree sample
 ******************************************************/

#include <iostream>
#include <limits>
#include <thrust/device_vector.h>
#include <thrust/extrema.h>
#include <thrust/iterator/zip_iterator.h>
#include <thrust/reduce.h>
#include <thrust/scan.h>
#include <thrust/sort.h>
#include <thrust/tuple.h>
#include <thrust/unique.h>
#include <string>

#include "chooseCard.cuh"
#include "directionEnums.hpp"
#include "errorMessages.hpp"
#include "geometry.hpp"
#include "gHull.cuh"
#include "pba2D.h"
#include "voronoi.hpp"

#define XBLOCKSIZE  32
#define YBLOCKSIZE  32

#define RES         512		// Resolution of digital projection

#define NFACES      6		// Number of faces of a cube...
#define DIM         3		// Dimensions
#define EPS         1e-5	// Epsilon
#define MAXSTARSIZE 10

// pba parameters, choice of parameters discussed in pba paper
// These are important, they determine the blocksizes
#define P1B	16		// Phase 1 band
#define P2B	16		// Phase 2 band
#define P3B	16		// Phase 3 band


// typedef thrust::device_vector dvec;
// typedef thrust::host_vector   hvec;
template <typename T> using dvec = thrust::device_vector<T>;
template <typename T> using hvec = thrust::host_vector<T>;

// Decided against namespace std becomes too many conflicts with thrust
using std::cout;
using std::endl;

// Trick to dynamically access float 3's
union float_type
{
  float3 data3;
  float data[3];
};


// Class for improving contiguous memory accessing
class Points {
  float *_x;
  float *_y;
  float *_z;

public:
  __host__ __device__ 
  Points() : _x(NULL), _y(NULL), _z(NULL) {};

  __host__ __device__ 
  Points(float *x, float *y, float *z) : _x(x), _y(y), _z(z) {};

  __host__ __device__ __forceinline__
  float3 get_point(int id) const {
    return make_float3(_x[id],_y[id],_z[id]);
  }

  __host__ __device__ __forceinline__ 
  void set_point(int id, const float3 &p) {
    _x[id] = p.x;
    _y[id] = p.y;
    _z[id] = p.z;
  }

  // Set the pointers. 
  __host__ __device__ __forceinline__ 
  void set(float *x, float *y, float *z) {
    _x = x;
    _y = y;
    _z = z;
  }
};

// Class for storing projections and IDS
class Face {
  float *data;
  int *pids;
  int length;

  __host__ __device__ __forceinline__
  int index(const int i, const int j) const {return j + i*length; }
public:
  __host__ __device__
  Face() : data{NULL}, pids{NULL}, length{0} {}
  
  __host__ __device__
  Face( float *data, int *ids, int length ) : data{data}, pids{ids}, length{length} {} 

  __host__ __device__ __forceinline__
  void set ( float *datas, int *ids, int L ) { data = datas; pids = ids; length = L; }
  
  __host__ __device__ __forceinline__
  float get_data ( int idx, int idy ) const { return data[index(idx,idy)]; }
  
  __host__ __device__ __forceinline__
  void set_data (int idx, int idy, float val) {  data[index(idx,idy)] = val; }
  
  __host__ __device__ __forceinline__
  float get_id ( int idx,int idy ) const { return pids[index(idx,idy)]; }
  
  // Value is the point id and id is the index
  __host__ __device__ __forceinline__
  void set_id (int idx,int idy, int val) {  pids[index(idx,idy)] = val; }  
  
  __host__ __device__ __forceinline__
  int get_length() { return length; }
};


// Class for improving contiguous memory accessing
class Stars {
  int *_ids;
  int *_edgeList;
  int *_sizes;
  int _nstars;
  int _maxsize;
  //  bool alive; Does size not have this role?

public:
  __host__ __device__ 
  Stars() : _ids(NULL), _edgeList(NULL), _sizes(NULL), _nstars(0), _maxsize(0) {};

  __host__ __device__ 
  Stars(int *ids, int *edgeList, int *sizes, int nstar, int maxsize ) :
    _ids(ids), _edgeList(edgeList), _sizes(sizes), _nstars(nstar), _maxsize(maxsize) {};

  // Set the pointers.                
  __host__ __device__ __forceinline__ 
  void set(int *ids, int *edgeList, int *sizes, int nstar, int maxsize ) {
    _ids = ids;
    _edgeList = edgeList;
    _sizes = sizes;
    _nstars = nstar;
    _maxsize = maxsize;
  }

  __host__ __device__ __forceinline__
  int get_id(int i) const {
    return _ids[i];
  }

  __host__ __device__ __forceinline__
  int *get_edgeList(int i) {
    return &_edgeList[_maxsize*i];
  }

  __host__ __device__ __forceinline__
  int get_size ( int i ) const {
    return _sizes[i];
  }
  
  __host__ __device__ __forceinline__
  void set_size ( int i, int sz ) {
    if ( sz > _maxsize ) { printf ( "STAR IS TOO BIG\n" ); asm("trap;"); }
    _sizes[i] = sz;
  }
  
  __host__ __device__ __forceinline__
  int nstars() const {
    return _nstars;
  }
};


template < typename vec > 
void fillHostArrays ( vec &px, vec &py, vec &pz, const CompGeom::Geometry & geom ) {
  size_t i=0;
  for ( const auto &p : geom ) {
    px[i] = p[0];
    py[i] = p[1];
    pz[i] = p[2];
    i++;
  }
}

// Debug kernel
__global__ 
void printPointsOnCard ( const Points *points, const int n_points ) {
  int id = threadIdx.x + blockIdx.x*blockDim.x;

  if ( id < n_points ) {
    float3 myp = points->get_point(id);
    printf ( "%d: %lf %lf %lf\n", id, myp.x, myp.y, myp.z ); 
  }
}

// Finds extrema on the device
// If the compiler is smart then the dereference won't transfer to host and then back
void findExtremes_d ( dvec<float> &extrm_d, 
		      const dvec<float> &px_d, 
		      const dvec<float> &py_d, 
		      const dvec<float> &pz_d ) 
{
  auto extx = thrust::minmax_element(px_d.begin(),px_d.end());
  auto exty = thrust::minmax_element(py_d.begin(),py_d.end());
  auto extz = thrust::minmax_element(pz_d.begin(),pz_d.end());
  extrm_d[Direction::LEFT ] = *extx.first;
  extrm_d[Direction::RIGHT] = *extx.second;
  extrm_d[Direction::BACK ] = *exty.first;
  extrm_d[Direction::FRONT] = *exty.second;
  extrm_d[Direction::DOWN ] = *extz.first;
  extrm_d[Direction::UP   ] = *extz.second;
}

__global__
void projectToFace_d ( Face *face, Points *points, float *ex, int dir, int N ) {
  const int w = face->get_length();
  const int i = dir %DIM;
  const int j = (i+1) %DIM;
  const int k = (i+2) %DIM;
  const float minw = ex[j];
  const float minh = ex[k];
  const float maxw = ex[j+DIM];
  const float maxh = ex[k+DIM];
  const float pos  = ex[dir];

  const int id = threadIdx.x + blockIdx.x*blockDim.x;
  
  // printf ( "%d\n", id );

  if ( id < N )  {
    float_type p;
    p.data3 = points->get_point(id);
    
    int idj = int(w*(p.data[j]-minw)/((maxw-minw)*(1+EPS)));
    int idk = int(w*(p.data[k]-minh)/((maxh-minh)*(1+EPS)));

    float       cmin_val = face->get_data(idj,idk);
    float       this_min = fabs(p.data[i]-pos);

    if ( cmin_val > this_min ) {
      face->set_data  (idj,idk, this_min   );
      face->set_id    (idj,idk, id         );
    }
  }
}

// Each thread spawns a projection kernel (dynamic parallelism 
__global__
void projectToBox_d ( Face *face_d, Points *points_d, float *extrm_d, int N ) {
  dim3 dimBlock ( XBLOCKSIZE );
  dim3 dimGrid ( (N/dimBlock.x) + (!(N%dimBlock.x)?0:1) );
  int dir = threadIdx.x;
  if ( dir < NFACES ) {
    projectToFace_d <<<dimGrid,dimBlock>>>( face_d+dir, points_d, extrm_d, dir, N );
  }
}

__host__ __device__
inline int shortID ( int i, int j ) {
  return 2*(i*RES + j );
}

// pba transposes the coordinates, hence idx and idy are swapped at assignment
__global__
void fillInput_d(short * input, Face * face ) {
  int idx   = blockIdx.x*blockDim.x+threadIdx.x;
  int idy   = blockIdx.y*blockDim.y+threadIdx.y;
  
  if ( idx < RES && idy < RES ) {
    if ( face->get_id(idx,idy) != MARKER ) {
      int index = shortID ( idx, idy );
      input[index]   = idy;
      input[index+1] = idx;
    }
  }
}

// Range based loops in CUDA
__global__
void countOutEdges_d(int *out_edges_p,short *V) {
  int idx   = blockIdx.x*blockDim.x+threadIdx.x;
  int idy   = blockIdx.y*blockDim.y+threadIdx.y;

  if ( idx < RES && idy < RES ) {
    int index  = idx*RES + idy;
    int sindex = 2*index;
    // int count  = 0;

    const int dir[4][2] =  {{-1,0},{0 ,-1},{0 ,1},{1 ,0}};
    for ( auto d : dir ) {
      int ni = idx+d[0];
      int nj = idy+d[1];
      
      int nindex = shortID(ni,nj);
      // Check bounds first 
      if ( !(ni < 0 || nj < 0 || ni >= RES || nj >= RES) 
      	   && (V[nindex] != V[sindex] || V[nindex+1] != V[sindex+1]) )
      	{
	  // count++;
	  out_edges_p[index]++;
	}
    }
    // out_edges_p[index] = max(0,count-2);
  }  
}

__global__
void constructWorkingSet_d(int *frsts,int*scnds,short *V, Face *face,int *out_edges_offest) {
  int idx   = blockIdx.x*blockDim.x+threadIdx.x;
  int idy   = blockIdx.y*blockDim.y+threadIdx.y;

  if ( idx < RES && idy < RES ) {
    int index  = idx*RES + idy;
    int sindex = 2*index;
    int count  = 0;

    const int dir[4][2] =  {{-1,0},{0 ,-1},{0 ,1},{1 ,0}};
    for ( auto d : dir ) {
      int ni = idx+d[0];
      int nj = idy+d[1];
      
      int nindex = shortID(ni,nj);
      // Check bounds first               
      if ( !(ni < 0 || nj < 0 || ni >= RES || nj >= RES) 
      	   && (V[nindex] != V[sindex] || V[nindex+1] != V[sindex+1]) )
      	{
	  frsts[out_edges_offest[index]+count] = face->get_id ( V[sindex+1], V[sindex] );   
	  scnds[out_edges_offest[index]+count] = face->get_id ( V[nindex+1], V[nindex] );   
	  count++;
	}
    }
    // If only one edge was added, then it's definitely double counting
    // if ( count == 1 ) {
    //   frsts[index*4] = frsts[index*4+1];   
    //   scnds[index*4] = scnds[index*4+1];   
    // }
  }  
}

// predicate functor for comparing edges on device
typedef thrust::tuple<int,int> edgeTuple;

struct compareEdges : public thrust::binary_function<edgeTuple,edgeTuple,bool> {
  __host__ __device__
  bool operator()(const edgeTuple &a, const edgeTuple &b) {
    return a.get<0>() != b.get<0>() ? a.get<0>() < b.get<0>() : a.get<1>() < b.get<1>();
  }
};


void constructVoronois_d( dvec<int> &firsts, dvec<int> &seconds, Face *face ) {

  const size_t RSQ = RES*RES;

  dvec<short> input ( 2*RSQ );
  std::vector < dvec<short> > output (3,dvec<short>(2*RSQ));

  dim3 dimBlock ( XBLOCKSIZE, YBLOCKSIZE  );
  dim3 dimGrid  ( (RES/dimBlock.x) + (!(RES%dimBlock.x)?0:1), 
		  (RES/dimBlock.y) + (!(RES%dimBlock.y)?0:1) );

  // Initialise memory for PBA algorithm on device
  pba2DInitialization(RES);

  for ( int i=0; i<3; i++ ) {
    short * input_p  = thrust::raw_pointer_cast(&input[0]) ;
    short * output_p = thrust::raw_pointer_cast(&output[i][0]);

    thrust::fill(input.begin(),input.end(),MARKER );
    fillInput_d <<< dimGrid,dimBlock >>> (input_p,face+i );
    // Don't know what the last three numbers do
    pba2DVoronoiDiagram_d(input_p,output_p,P1B,P2B,P3B); 

    hvec<short> input_h = input, output_h = output[i];
    makeVoronoiPBM ( &output_h[0], &input_h[0], RES,RES, 
		     "parallel" + std::to_string(i) + ".pbm" );
  }

  // Free PBA memory
  pba2DDeinitialization();

  /////////// This was an attempt at finding the dual using a prefix sum

  dvec<int> n_out_edges(6*RSQ,0);
  for ( int i=0; i<3; i++ ) {
    short * output_p    = thrust::raw_pointer_cast(&output[i][0]);
    int   * out_edges_p;

    out_edges_p = thrust::raw_pointer_cast(&n_out_edges[i*RSQ]);
    countOutEdges_d<<<dimBlock,dimGrid>>>(out_edges_p,output_p);

    out_edges_p = thrust::raw_pointer_cast(&n_out_edges[(i+DIM)*RSQ]);
    countOutEdges_d<<<dimBlock,dimGrid>>>(out_edges_p,output_p);    
  }

  //Find the offsets for each array plus the output size
  size_t sz = n_out_edges.back();
  thrust::exclusive_scan(n_out_edges.begin(),n_out_edges.end(),n_out_edges.begin());
  sz += n_out_edges.back();

  // cout << "Size of initial triangulation: " << sz << endl; 
  firsts  = dvec< int > (sz,std::numeric_limits<int>::max());
  seconds = dvec< int > (sz,std::numeric_limits<int>::max());

  // for ( int i=0; i<3; i++ ) {
  //   short * output_p    = thrust::raw_pointer_cast(&output[i][0]);
  //   int   * firsts_p    = thrust::raw_pointer_cast(&firsts[0]   );
  //   int   * seconds_p   = thrust::raw_pointer_cast(&seconds[0] );
  //   int   * out_edges_p;

  //   out_edges_p = thrust::raw_pointer_cast(&n_out_edges[i*RSQ]);
  //   // countOutEdges_d<<<dimBlock,dimGrid>>>(out_edges_p,output_p);

  //   out_edges_p = thrust::raw_pointer_cast(&n_out_edges[(i+DIM)*RSQ]);
  //   // countOutEdges_d<<<dimBlock,dimGrid>>>(out_edges_p,output_p);    
  // }

  //////////////////////////////// CONSTRUCT WORKING SETS ////////////////////////////////

  //////// I just allocated the maximum possible need memory and then reduce it at the end

  // firsts  = dvec<int>(6*4*RES*RES,std::numeric_limits<int>::max());  
  // seconds = dvec<int>(6*4*RES*RES,std::numeric_limits<int>::max());  
  for ( int i=0; i<3; i++ ) {
    short * output_p    = thrust::raw_pointer_cast(&output[i][0]);
    int   * firsts_p;
    int   * seconds_p;
    int   * out_edges_p;

    // All the min faces
    firsts_p    = thrust::raw_pointer_cast(&firsts[0]   );
    seconds_p   = thrust::raw_pointer_cast(&seconds[0]  );
    out_edges_p = thrust::raw_pointer_cast(&n_out_edges[i*RSQ]);
    constructWorkingSet_d<<<dimBlock,dimGrid>>> (firsts_p,
						 seconds_p,
						 output_p,
						 &face[i],
						 out_edges_p);

    // All the max faces
    // firsts_p    = thrust::raw_pointer_cast(&firsts[(i+DIM)*4*RSQ]   );
    // seconds_p   = thrust::raw_pointer_cast(&seconds[(i+DIM)*4*RSQ]  );
    out_edges_p = thrust::raw_pointer_cast(&n_out_edges[(i+DIM)*RSQ]);
    constructWorkingSet_d<<<dimBlock,dimGrid>>> (firsts_p,
						 seconds_p,
						 output_p,
						 &face[i+DIM],
						 out_edges_p);
  }  
  
  //////////////////// SORT AND REMOVE DUPLICATES ////////////////////////////////

  auto edgesStart 
    = thrust::make_zip_iterator ( thrust::make_tuple ( firsts.begin(), 
						       seconds.begin() ) );
  auto edgesEnd   
    = thrust::make_zip_iterator ( thrust::make_tuple ( firsts.end(),   
						       seconds.end()   ) );

  // cout << "Before Removing Markers      : " 
  //      << thrust::distance ( edgesStart,edgesEnd ) << endl;
  // edgesEnd = thrust::remove ( edgesStart, edgesEnd, 
  // 			      thrust::tuple<int,int> (std::numeric_limits<int>::max(),
  // 						      std::numeric_limits<int>::max()));

  // Remove some duplicates before sorting
  // cout << "Before Removing Duplicates 1 : " 
  //      << thrust::distance ( edgesStart,edgesEnd ) << endl;
  // edgesEnd = thrust::unique ( edgesStart, edgesEnd );
  thrust::sort ( edgesStart, edgesEnd  , compareEdges() );
  // cout << "Before Removing Duplicates 2 : " 
  //      << thrust::distance ( edgesStart,edgesEnd ) << endl;
  edgesEnd = thrust::unique ( edgesStart, edgesEnd );

  firsts.resize(thrust::distance ( edgesStart,edgesEnd ));
  seconds.resize(thrust::distance ( edgesStart,edgesEnd ));

  // cout << "Final Size : " << firsts.size() << endl;
  // dvec<int> star_count
  // for ( auto it = edgesStart; it != edgesEnd; ++it ) {
  //   cout << thrust::get<0>(*it) << " " << thrust::get<1>(*it) << endl;
  // }
}

// Overload the difference operator for float3
__device__
float3 operator-(float3 a, float3 b) {
  return make_float3 ( a.x - b.x, a.y - b.y, a.z - b.z );
}

// Calculates the normal vector of a float3
__device__
inline float3 norm ( float3 tri[3] ) {
  float3 a = tri[1] - tri[0], b = tri[2] - tri[0]; 
  return make_float3 ( a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.y, a.x*b.y - a.y*b.x );
}

// Determines if p is visible to tri
__device__
inline bool isVisible (float3 tri[3], float3 p) {
  float3 n   = norm ( tri );
  // float3 com = average(tri);
  // float3 v = com - p;
  // dot product
  return true;
}

__global__
void constructStars_Kernel ( Stars *stars, 
			     int* seconds, 
			     int * scnds_offsets, 
			     Points *points ) 
{ 
  int idx   = blockIdx.x*blockDim.x+threadIdx.x;

  if ( idx < stars->nstars() ) {
    int  myid = stars->get_id(idx);
    int *myws = &seconds[scnds_offsets[idx]];
    int  mysz = stars->get_size(idx);
    int *myel = stars->get_edgeList(idx);
    
    float3 tri[3];
    tri[0]   = points->get_point(myid);
    tri[1]   = points->get_point(myws[0]);
    tri[2]   = points->get_point(myws[1]);

    float3 p = points->get_point(myws[2]);

    if ( isVisible ( tri, p ) ) 
      ;; //Then invert

  }
}

static void constructStars_d ( Stars *stars_d, 
			       dvec<int> &seconds, 
			       dvec<int> &scnds_offsets, 
			       Points *points_d )
{
  int n_stars = scnds_offsets.size();

  dim3 dimBlock ( XBLOCKSIZE );
  dim3 dimGrid ( (n_stars/dimBlock.x) + (!(n_stars%dimBlock.x)?0:1) );  

  constructStars_Kernel <<< dimGrid, dimBlock >>> 
    ( stars_d,
      thrust::raw_pointer_cast(&seconds[0]),
      thrust::raw_pointer_cast(&scnds_offsets[0]),
      points_d );
}


std::vector < std::vector < size_t > > gHull ( const CompGeom::Geometry &geom ) {

  // chooseCudaCard(0);
  gpuErrchk ( cudaGetLastError() );

  size_t N = geom.size();

  /////////////////////////////// INIT POINTS ON DEVICE /////////////////////////////////
  
  // Fill three 1d arrys with the geometry
  hvec<float> px_h(N), py_h(N), pz_h(N);
  fillHostArrays ( px_h, py_h, pz_h , geom   );

  // Transfer points to card
  dvec<float> px_d = px_h, py_d = py_h, pz_d = pz_h;

  //  Initialise geometry class on card
  //  Pass the device vectors rather than the host vectors
  Points points_h (thrust::raw_pointer_cast(&px_d[0]), 
		   thrust::raw_pointer_cast(&py_d[0]), 
		   thrust::raw_pointer_cast(&pz_d[0]) );
  Points *points_d;
  cudaMalloc ( (void **) &points_d, sizeof(Points) );
  cudaMemcpy ( points_d, &points_h, sizeof(Points), cudaMemcpyHostToDevice );

  /////// PRINT POINTS ON CARD //////////////
  // dim3 dimBlock ( XBLOCKSIZE );
  // dim3 dimGrid ( (N/dimBlock.x) + (!(N%dimBlock.x)?0:1) );
  // printPointsOnCard <<<dimGrid,dimBlock>>> ( points_d, N );
  
  ////////////////////////////////// SETUP FACES ////////////////////////////////////

  //Init faces
  // Numric limits may be different on device, be careful!
  std::vector < dvec<float> > datas_d ( NFACES,  dvec<float> (RES*RES,std::numeric_limits<float>::max()) );
  std::vector < dvec<int  > > pids_d  ( NFACES,  dvec<int  > (RES*RES, MARKER) );

  Face face_h[NFACES];
  Face *face_d;
  for ( size_t i=0; i<NFACES; i++ ) {
    face_h[i].set (thrust::raw_pointer_cast(&datas_d[i][0]),
		   thrust::raw_pointer_cast(&pids_d[i][0]), 
		   RES );
  }
  cudaMalloc ( (void **) &face_d, NFACES*sizeof(Face) );
  cudaMemcpy ( face_d, &face_h, NFACES*sizeof(Face), cudaMemcpyHostToDevice );

  ////////////////////////////////// FIND EXTREMA /////////////////////////////////

  // Find mins and maxs
  // Might be faster to do this on the CPU
  dvec<float> extrm_d(6);
  findExtremes_d ( extrm_d, px_d, py_d, pz_d );

  ////////////////////////////////// PROJECTION //////////////////////////////////
  
  projectToBox_d<<<1,NFACES>>> ( face_d, 
				 points_d, 
				 thrust::raw_pointer_cast(&extrm_d[0]), 
				 N );

  ////////////////////////////////// CONSTRUCT VORONOIS //////////////////////////

  // This actually finds all the edges
  dvec< int > firsts;
  dvec< int > seconds;
  constructVoronois_d ( firsts,seconds,face_d );

  ////////////////////////////////// MAKE STARS //////////////////////////

  dvec<int> star_ids   ( N                );
  dvec<int> star_sizes ( firsts.size() ,1 );

  // Find the unique ids and their counts
  auto key_val_end = thrust::reduce_by_key ( firsts.begin()    , 
					     firsts.end()      , 
					     star_sizes.begin(), 
					     star_ids.begin()  ,
					     star_sizes.begin() );

  // for ( auto it = star_ids.begin(), it2 = star_sizes.begin(); 
  // 	it != key_val_end.first; 
  // 	++it, ++it2 ) 
  //   {
  //     cout << *it << ": " << *it2 << endl;
  //   } 
  
  int n_stars = thrust::distance ( star_ids.begin(), key_val_end.first );
  
  dvec<int> edgeList ( MAXSTARSIZE * n_stars ) ;
  dvec<int> scnds_offsets ( n_stars );
  thrust::exclusive_scan ( star_sizes.begin(), star_sizes.begin() + n_stars, scnds_offsets.begin() );
  
  Stars stars_h (thrust::raw_pointer_cast(&star_ids[0]),
		 thrust::raw_pointer_cast(&edgeList[0]),
		 thrust::raw_pointer_cast(&star_sizes[0]),
		 n_stars,
		 MAXSTARSIZE );

  Stars *stars_d;
  cudaMalloc ( (void **) &stars_d, sizeof(Stars) );
  cudaMemcpy ( stars_d, &stars_h, sizeof(Stars), cudaMemcpyHostToDevice );

  constructStars_d ( stars_d, seconds, scnds_offsets, points_d );
  
  ////////////////////////////////// DEBUGGING   /////////////////////////////////
  
  // Transfer back for debugging
  // std::vector < hvec<float> > datas_h(datas_d.begin(),datas_d.end());
  // std::vector < hvec<int  > > pids_h (pids_d.begin(),pids_d.end());
  // for ( auto dat : datas_h ) {
  //   for ( int i=0; i<RES*RES; i++ ) {
  //     if (i%10==0 && i!=0) cout << endl;
  //     cout << dat[i] << " " ;
  //   }
  //   cout << endl << endl;;
  // }

  // for ( auto dat : pids_h ) {
  //   for ( int i=0; i<RES*RES; i++ ) {
  //     if (i%10==0 && i!=0) cout << endl;
  //     cout << dat[i] << " " ;
  //   }
  //   cout << endl << endl;;
  // }

  // Copy back
  // px_h = px_d;
  // py_h = py_d;
  // pz_h = pz_d;

  // for ( size_t i=0; i<px_h.size(); i++ ) {
  //   cout << geom[i] << endl;
  //   cout << px_h[i] << " " << py_h[i] << " " << pz_h[i] << endl << endl;
  // }
  
  cudaFree ( points_d );
  cudaFree ( face_d  );
  cudaFree ( stars_d );
  gpuErrchk ( cudaGetLastError() );
  
  return {{1}};
}
