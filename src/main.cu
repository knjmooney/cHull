/******************************************************
 * Name    : main.cpp
 * Author  : Kevin Mooney
 * Created : 13/06/16
 * Updated : 25/07/16
 *
 * Description:
 *
 *
 * NOTES:
 *  Graham Scan has a bug when executing the final loop
 ******************************************************/

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>		// unix standard header file

#include "convexHull2D.hpp"
#include "insertion3D.hpp"
#include "gHull.cuh"
#include "gHullSerial.hpp"
#include "cudaHull.hpp"		// 2D convex hull on GPU
#include "errorMessages.hpp"
#include "geometry.hpp"
#include "workingSet.hpp"
#include "star.hpp"

using namespace std;
using namespace std::chrono;

// DEBUGGING //
CompGeom::Star constructStar_h ( const CompGeom::WorkingSet & W, const CompGeom::Geometry &geom );
///////////////


// Macro for timing function calls
// Could rewrite as seperate functions like start and stop timer
#define timer(fun) {							 \
    chrono::steady_clock::time_point t1 = chrono::steady_clock::now();	\
    (fun);								 \
    chrono::steady_clock::time_point t2 = chrono::steady_clock::now();	\
    chrono::duration<float> time_span = duration_cast<duration<float>>(t2 - t1); \
    printf("%-20s: %lf\n",#fun, time_span.count());			 \
}

// Prints a usage message when -h flag is passed
// Could try integrate usage message with getopts
void printUsage() {
  string params[][2] = {{"-a arg","List of algorithms to be run                        "},
			{""      ,"Make sure all algorithm names are comma deliminated "},
			{""      ,"Options are :                                       "},
			{""      ,"  - grahamScan  (2D)                                "},
			{""      ,"  - giftWrap    (2D)                                "},
			{""      ,"  - cudaHull    (2D)                                "},
			{""      ,"  - insertion   (3D)                                "},
			{""      ,"  - gHullSerial (3D)                                "},
			{""      ,"  - gHull       (3D) (cuda)                         "},
			{""      ,"                                                    "},
			{"-d arg","Set the dimension                                   "},
			{"-h"    ,"Prints this help message and exits succesfully      "},
                        {"-f arg","Prints config to $arg                               "},
                        {"-t"    ,"Prints the time taken by each function              "}};
  printf("Usage: ./%s [options] ...\n",__FILE__);
  printf("Options:\n"                          );
  for ( const auto &str : params ) {
    printf("  %-25s %s\n",str[0].c_str(),str[1].c_str());
  }
}

int main(int argc, char *argv[]) {

  // DEBUGGING
  // {
  // CompGeom::Geometry geom =  { {0,0,0}, {0,-1,0}, {1,0,0}, {0,0,1}, {-1,0,0},
  // 				 {0,1,0}, {0,0,-1}, {0,0,0}, {2,0,0}, {0,0,2}, {-2,0,0},
  // 				 {0,2,0}, {0,0,-2}, {-2,0,2}, {0,0,3} };
  //   CompGeom::Geometry geom =  { {0,0,0}, {0,-1,0}, {1,0,0}, {0,0,1}, {-1,0,0},
  //   				 {0,1,0}, {0,0,-1} };
  //   gHull(geom);
  //   gHullSerial(geom);
  // }


  // Set defaults
  string algorithms    = "giftWrap";
  string filename      = "";
  bool time_func_calls = 0;
  size_t dim           = 2;
  size_t n_points      = 10;
 
  // Parse command line
  int option;
  while ((option = getopt (argc, argv, "a:d:hf:n:t")) != -1) {
    switch(option) {
    case 'a':
      algorithms = optarg;
      break;
    case 'd':
      dim        = atoi(optarg);
      break;
    case 'h':
      printUsage();
      return EXIT_SUCCESS;
    case 'f':
      filename   = optarg;
      break;
    case 'n':
      n_points   = atol(optarg);
      break;
    case 't':
      time_func_calls = 1;
      break;
    default:
      printUsage();
      return EXIT_FAILURE;
    }
  }

  CompGeom::Geometry geom{dim};
  geom.addRandom(n_points);

  std::istringstream ss(algorithms);
  std::string token;

  while(std::getline(ss, token, ',')) {
    if ( token == "insertion" ) {

      // else if is easier to read than nested if statements 
      if ( filename == "" && time_func_calls ) {
	timer ( insertion3D(geom) );
      }    
      else if ( filename == "" ) {
	insertion3D(geom);	// This is pointless and has no output
      }
      else if ( time_func_calls ) {
	timer ( insertion3D(geom,filename) );
      } 
      else
	insertion3D(geom,filename);
    }

    else if ( token == "gHullSerial" ) {
      if ( time_func_calls ) {
	timer ( gHullSerial(geom) );
      }
      else 
	gHullSerial(geom);
    }    

    else if ( token == "giftWrap" ) {
      if ( time_func_calls ) {
	timer ( giftWrap(geom) );
      }
      else giftWrap(geom) ;
    }    

    else if ( token == "grahamScan" ) {
      if ( time_func_calls ) {
	timer ( grahamScan(geom) );
      }
      else grahamScan ( geom );
    }    
    else if ( token == "cudaHull" ) {
      if ( time_func_calls ) {
      	timer ( cudaHull(geom) );
      }
      else cudaHull ( geom );
    }    
    else if ( token == "gHull" ) {
      if ( time_func_calls ) {
      	timer ( gHull(geom) );
      }
      else gHull ( geom );
    }        

    else {
      cerr << "Don't recognise algorithm " << token << endl;
    }
  }

  return EXIT_SUCCESS;
}
