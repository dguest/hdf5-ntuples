#ifndef TOOLS_HH
#define TOOLS_HH

#include "H5Cpp.h"
#include <string>
#include <vector>

template<typename T>
class OneDimBuffer
{
public:
  OneDimBuffer(std::string ds_name, const H5::DataType& type, int max = 10);
private:
  std::vector<T> _buffer;
};

#endif
