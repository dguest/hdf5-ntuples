// Example showing how to write a one-dimensional dataset.
//
// In this case we're saving one compound type per entry, which is
// pretty close to a standard HEP ntuple.

// Writing on every entry is slow, so we buffer entries and then write
// them as blocks. The OneDimBuffer handles this (although with some
// caveats with variable-length containers).
#include "OneDimBuffer.hh"

#include <vector>
#include <iostream>
#include <cstddef>
#include <cmath>
#include "H5Cpp.h"
#include "h5container.hh"

// _________________________________________________________________________
// output structure

// This dummy `Entry` structure could correspond to an event, a jet, etc
struct Entry {
  double value_d;
  int value_i;
  const char* value_s;

  // `hvl_t` is a variable-length type that hdf5 can recognize.
  h5::vector<double> vector_d;

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

  // Build the output file. Since multiple writes happen throughout
  // the run we have to build this _before_ looping through entries.
  //
  // I'm using HDF_ACC_TRUNC here, which tells HDF5 to overwrite
  // existing files. It's safer to use H5F_ACC_EXCL, which will throw
  // an exception rather than overwrite.
  H5::H5File file("test.h5", H5F_ACC_TRUNC);

  // Instance one example buffer. We'll call this one `data` and have
  // it store one integer per event.
  OneDimBuffer<int> int_buffer(file, "some_ints", itype);

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
  entryType.insertMember("value_s", offsetof(Entry, value_s), stype);
  entryType.insertMember("vector_d", offsetof(Entry, vector_d.h5), vl_dtype);
  OneDimBuffer<Entry> ebuffer(file, "entries", entryType);

  // Now we generate some dummy data
  for (int iii = 0; iii < 98; iii++) {
    // the int_buffer is easy: just push back.
    int_buffer.push_back(iii);

    // Create a vector of doubles to store.
    std::vector<double> d_vect;
    for (int jjj = 0; jjj < (iii % 10); jjj++) {
      d_vect.push_back(jjj / double(iii + 1));
    }
    // Create a dummy string.
    std::string some_string("this is " + std::to_string(iii));

    // Compose these things into an `Entry` object.
    Entry entry{
      std::sqrt(iii),
	iii*2,
	some_string.data(),
	d_vect};
    // add the `Entry` to the buffer.
    ebuffer.push_back(entry);

    // FIXME: this is super-annoying, but we have to flush the buffer
    //        (i.e. write) with every entry.  The reason is that the
    //        `data()` calls above return pointers, which aren't valid
    //        once `d_vect` or `some_string` go out of scope.
    //
    //        I'm considering wrapping some vector types in a class
    //        that owns a `hvl_t` and updates the pointer on copy /
    //        move / etc. That way we can stuff vectors into
    //        structures and buffer them like we would any other
    //        primative type. Will do that if any of this turns out to
    //        be useful...
    // ebuffer.flush();
  }
  // Buffers need to be flushed after the loop, since `flush` is only
  // called automatically when the buffer fills up.
  int_buffer.flush();
  ebuffer.flush();

}
