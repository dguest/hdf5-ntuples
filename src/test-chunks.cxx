#include "OneDimBuffer.hh"

#include <vector>
#include <iostream>
#include <cstddef>
#include <cmath>
#include "H5Cpp.h"

struct Entry {
  double value_d;
  int value_i;
  const char* value_s;
  hvl_t vector_d;
  double some_shit[56];
};

// class EntryOwner: public Entry
// {
//   EntryOwner(
// }

int main(int argc, char* argv[]) {
  auto itype = H5::PredType::NATIVE_INT;
  auto dtype = H5::PredType::NATIVE_DOUBLE;
  auto stype = H5::StrType(H5::PredType::C_S1, H5T_VARIABLE);
  auto vl_dtype = H5::VarLenType(&dtype);

  H5::H5File file("test.h5", H5F_ACC_TRUNC);
  OneDimBuffer<int> buffer(file, "data", itype);

  H5::CompType entryType(sizeof(Entry));
  entryType.insertMember("value_d", offsetof(Entry, value_d), dtype);
  entryType.insertMember("value_i", offsetof(Entry, value_i), itype);
  entryType.insertMember("value_s", offsetof(Entry, value_s), stype);
  entryType.insertMember("vector_d", offsetof(Entry, vector_d), vl_dtype);
  OneDimBuffer<Entry> ebuffer(file, "entries", entryType);

  for (int iii = 0; iii < 98; iii++) {
    buffer.push_back(iii);
    std::vector<double> things;
    for (int jjj = 0; jjj < (iii % 10); jjj++) {
      things.push_back(jjj / double(iii + 1));
    }
    std::string some_string("this is " + std::to_string(iii));
    Entry entry{
      std::sqrt(iii),
	iii*2,
	some_string.data(),
	{things.size(), things.data()}};
    ebuffer.push_back(entry);
    ebuffer.flush();
  }
  buffer.flush();
  ebuffer.flush();

}
