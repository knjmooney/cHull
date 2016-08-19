/******************************************************
 * Name    : directionEnums.hpp
 * Author  : Kevin Mooney
 * Created : 16/08/16
 * Updated : 
 *
 * Description:
 *   The following is guaranteed
 *      LEFT % DIM = RIGHT % DIM
 *   and have the range [0,6)
 *
 * NOTES:
 *   Unsure about using invalid as an option
 ******************************************************/

#pragma once

#include <vector>

namespace Direction {
  enum Dir { LEFT = 0, BACK = 1, DOWN = 2, RIGHT = 3, FRONT = 4, UP = 5 };
  enum Orientation { XDIR = 0, YDIR = 1, ZDIR = 2, INVALID = 9999 };
  
  inline std::vector < Dir > allDirections() { 
    return std::vector < Dir > {{LEFT,BACK,DOWN,RIGHT,FRONT,UP}};
  } 
}
