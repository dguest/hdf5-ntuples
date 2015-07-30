// Example showing how to write a one-dimensional dataset.
//
// In this case we're saving one compound type per entry, which is
// pretty close to a standard HEP ntuple.

// Writing on every entry is slow, so we buffer entries and then write
// them as blocks. The OneDimBuffer handles this (although with some
// caveats with variable-length containers).
#include "OneDimBuffer.hh"

#include "H5Cpp.h"
#include "h5container.hh"

#include <vector>
// #include <iostream>
#include <cstddef>
#include <cmath>

// _________________________________________________________________________
// output structure

// This dummy `Entry` structure could correspond to an event, a jet, etc
struct Entry {
  double value_d;
  int value_i;
  h5::string value_s;

  // Variable length containers have to be stored in special vectors.
  // Internally these wrap a hvl_t structure.
  h5::vector<double> vector_d;
  h5::vector<h5::string> vector_s;
  h5::vector<h5::vector<int> > vv_i;

  // Include a dummy field. This is just here to make sure it gets
  // stripped off when we run `pack()`.
  double some_stuff[56];
};

// _________________________________________________________________________
// main routine starts here

int main(int argc, char* argv[]) {
  // Define some shorthand for the `type' of the objects we're going to
  // write out.  HDF5 uses a lot of blind casts from void*, so getting
  // these wrong means segfault or corrupted data!
  //
  // `itype` and `dtype` should be pretty straightforward.
  auto itype = H5::PredType::NATIVE_INT;
  auto dtype = H5::PredType::NATIVE_DOUBLE;
  // `stype` is a string.
  auto stype = H5::StrType(H5::PredType::C_S1, H5T_VARIABLE);
  // `vl_dtype` is a variable-length double.
  // The `VarLenType` constructor takes a _pointer_ to the base datatype.
  auto vl_dtype = H5::VarLenType(&dtype);
  auto vl_itype = H5::VarLenType(&itype);
  auto vl_stype = H5::VarLenType(&stype);
  // We can do as many levels of nesting as we want.
  auto vvl_itype = H5::VarLenType(&vl_itype);

  // Build the output file. Since multiple writes happen throughout
  // the run we have to build this _before_ looping through entries.
  //
  // I'm using HDF_ACC_TRUNC here, which tells HDF5 to overwrite
  // existing files. It's safer to use H5F_ACC_EXCL, which will throw
  // an exception rather than overwrite.
  H5::H5File file("test.h5", H5F_ACC_TRUNC);

  // Instance one example buffer. We'll call this one `data` and have
  // it store one integer per event.
  const size_t buffer_size = 100;
  OneDimBuffer<int> int_buffer(file, "some_ints", itype, buffer_size);

  // We're more interested in storing compound types. These can
  // contain any collection of int, float, strings, or `hvl_t`
  // (variable length) types.
  //
  // The ugly c-style code below is to tell HDF5 where to look for
  // entries, since we're going to pass the HDF5 API a void*
  // corresponding to the array of `Entry` objects. This throws away
  // type info so we have to tell HDF5 what type we're working with.
  // You can change the name of the subtype (first argument in
  // `insertMember`), but be careful with the others.
  H5::CompType entryType(sizeof(Entry));
  entryType.insertMember("value_d", offsetof(Entry, value_d), dtype);
  entryType.insertMember("value_i", offsetof(Entry, value_i), itype);
  // Note that for variable length types we need to use special containers
  // each of these has an `h5` member which HDF5 can recognize.
  entryType.insertMember("value_s", offsetof(Entry, value_s.h5), stype);
  entryType.insertMember("vector_d", offsetof(Entry, vector_d.h5), vl_dtype);
  entryType.insertMember("vector_s", offsetof(Entry, vector_s.h5), vl_stype);
  entryType.insertMember("vv_i", offsetof(Entry, vv_i.h5), vvl_itype);
  OneDimBuffer<Entry> ebuffer(file, "entries", entryType, buffer_size);

  // Now we generate some dummy data
  for (int iii = 0; iii < 500; iii++) {
    // the int_buffer is easy: just push back.
    int_buffer.push_back(iii);

    // Create a few vectors to store
    std::vector<double> d_vect;
    h5::vector<h5::string> s_vect;
    h5::vector<h5::vector<int>> ivv;
    for (int jjj = 0; jjj < (iii % 10); jjj++) {
      d_vect.push_back(jjj / double(iii + 1));
      h5::vector<int> iv;
      for (int kkk = 0; kkk < jjj; kkk++) {
	iv.push_back(kkk);
      }
      ivv.push_back(iv);
      s_vect.push_back(std::to_string(jjj));
    }
    // Create a dummy string.
    std::string some_string("this is " + std::to_string(iii));

    // Compose these things into an `Entry` object.
    Entry entry{
      std::sqrt(iii),
    	iii*2,
    	some_string,
    	d_vect,
	s_vect,
	ivv};
    // add the `Entry` to the buffer.
    ebuffer.push_back(entry);
  }
  // Buffers need to be flushed after the loop, since `flush` is only
  // called automatically when the buffer fills up.
  int_buffer.flush();
  ebuffer.flush();

}
