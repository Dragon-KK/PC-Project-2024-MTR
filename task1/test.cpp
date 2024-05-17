#include <algorithm>
#include "Tank.h"
#include <iostream>

int main() 
{
  // The DataType can be custom as well, so do not hardcode for all primitive types
  using DataType = double;

  Tank<DataType> v(0);

  /*
    any value can be returned for out of bounds access, 
    test cases will not have out of bounds access
  */
  // DataType x = v[5];    // access 6th element of tank, consider 0 indexing
  v.push_back(1);
  DataType x = v.at(0); // access 1st element of tank, consider 0 indexing
  v.pop_back();
  std::size_t len = v.size(); // returns number of elements of tank

  v.push_back(x); // pushes x to the end of the tank
  v.pop_back(); // removes the last element of the tank (Assume that popback is called when tank is non-empty)

  // I'm assuming you want us to define a function that does this but idk why tf you would want that
  std::cout << accumulate(v) << std::endl; // sum of all elements in the tank
}