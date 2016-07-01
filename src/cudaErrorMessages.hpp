/******************************************************
 * Name    : cudaErrorMessages.hpp
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

// macro for checking CUDA errors
// template stolen from stack exchange
#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line )
{
  if (code != cudaSuccess) {
    std::ostringstream oss;
    oss << cudaGetErrorString(code) << " " <<  file << " " <<  line;
    throw std::logic_error ( oss.str() );
  }
}
