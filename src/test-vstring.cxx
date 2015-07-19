#include <vector>
#include <string>

#include "H5Cpp.h"


int main(int argc, char* argv[]) {
  // generate some data
  std::vector<std::string> strings;
  for (int iii = 0; iii < 10; iii++) {
    strings.push_back("this is " + std::to_string(iii));
  }

  // grab the pointers to the strings
  std::vector<const char*> chars;
  for (const auto& str: strings) {
    chars.push_back(str.data());
  }
  // create a variable length array, containing these pointers
  // be careful here, it casts to void*, make sure you get the type right!
  hvl_t hdf_buffer;
  hdf_buffer.p = chars.data();
  hdf_buffer.len = chars.size();

  // create the type, this is where the magic happens
  // start with a string
  auto s_type = H5::StrType(H5::PredType::C_S1, H5T_VARIABLE);
  s_type.setCset(H5T_CSET_UTF8); // just for fun, you don't need this
  // make a variable length type (to correspond to the vector of strings)
  auto svec_type = H5::VarLenType(&s_type);

  // create the output
  H5::H5File out_file("vtest.h5", H5F_ACC_EXCL);
  // from the dataspace point of view this is just a scalar: no
  // fancy dataspace needed.
  H5::DataSet dataset(
    out_file.createDataSet("the_ds", svec_type, H5S_SCALAR));
  dataset.write(&hdf_buffer, svec_type);

  return 0;
}
