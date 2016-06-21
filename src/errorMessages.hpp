/******************************************************
 * Name    : errorMessages.hpp
 * Author  : Kevin Mooney
 * Created : 17/06/16
 * Updated :
 *
 * Description:
 *    Macros for handeling errors 
 ******************************************************/

#pragma once

#include <stdexcept>
#include <sstream>

#define errorM(error) { printError((error),__FILE__,__LINE__); }
inline void printError(const char *str,const char *file, int line ) {
  std::ostringstream oss;
  oss << file << ":" <<  line << ": " << str;
  throw std::logic_error ( oss.str() );
}
