/******************************************************
 * Name    : errorMessages.hpp
 * Author  : Kevin Mooney
 * Created : 17/06/16
 * Updated : 01/07/16
 *
 * Description:
 *    Macros for handeling errors 
 ******************************************************/

#pragma once

#include <stdexcept>
#include <sstream>

// macro for printing error messages with file and line number
#define errorM(error) { printError((error),__FILE__,__LINE__); }
inline void printError(const char *str,const char *file, int line ) {
  std::ostringstream oss;
  oss << file << ":" <<  line << ": " << str;
  throw std::logic_error ( oss.str() );
}

// Macro to be wrapped around Cuda Calls to handle errors correctly
#ifdef __CUDACC__		// if compiling cuda
#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line )
{
  if (code != cudaSuccess) {
    std::ostringstream oss;
    oss << cudaGetErrorString(code) << " " <<  file << " " <<  line;
    throw std::logic_error ( oss.str() );
  }
}
#endif
