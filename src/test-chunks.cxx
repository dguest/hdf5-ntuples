#include "OneDimBuffer.hh"

#include <vector>
#include <iostream>
#include "H5Cpp.h"

int main(int argc, char* argv[]) {
  H5::H5File file("test.h5", H5F_ACC_TRUNC);
  auto dtype = H5::PredType::NATIVE_INT;

  OneDimBuffer<int> buffer(file, "data", dtype);
  for (int iii = 0; iii < 98; iii++) {
    buffer.push_back(iii);
    buffer.flush();
  }
}
