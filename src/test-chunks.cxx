// Example showing how to write a one-dimensional dataset.
//
// In this case we're saving one compound type per entry, which is
// pretty close to a standard HEP ntuple.

// Writing on every entry is slow, so we buffer entries and then write
// them as blocks. The OneDimBuffer handles this, although with some
// caveats for variable-length containers.
#include "OneDimBuffer.hh"
// The the `vector` defined in `h5container.hh` takes care of some of
// the interesting issues which arise when dealing with variable-length
// arrays. As a rule, use these vectors to write out data.
#include "h5container.hh"

#include "H5Cpp.h"

#include <vector>
#include <iostream>
#include <cstddef>
#include <cmath>

// _________________________________________________________________________
// Part 1: define output structures

// Our output structure will have a variable-length array of ``tracks''
struct Track {
  double pt;
  double eta;
};

struct MultiTrack {
  h5::vector<Track> tracks;
  int n_tracks;
};

// This dummy `Entry` structure could correspond to an event, a jet, etc
struct Entry {
  h5::vector<MultiTrack> verts;
};

// _________________________________________________________________________
// Part 2: define output types

H5::CompType getEntryType() {
  // Define some shorthand for the `type' of the objects we're going to
  // write out.  HDF5 uses a lot of blind casts from void*, so getting
  // these wrong means segfault or corrupted data!
  //
  // `itype` and `dtype` should be pretty straightforward.
  auto itype = H5::PredType::NATIVE_INT;
  auto dtype = H5::PredType::NATIVE_DOUBLE;

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
  //
  // Just for fun, add a vector of compound types to the `Entry`
  // structure.

  using h5::offset;
  using h5::simple_offset;
  H5::CompType trackType(sizeof(Track));
  trackType.insertMember("pt", simple_offset(&Track::pt), dtype);
  trackType.insertMember("eta", simple_offset(&Track::eta), dtype);
  auto tracksType = H5::VarLenType(&trackType);

  H5::CompType multitrack(sizeof(MultiTrack));
  multitrack.insertMember("tracks", offset(&MultiTrack::tracks),
			  tracksType);
  multitrack.insertMember("n_tracks", simple_offset(&MultiTrack::n_tracks),
  			  itype);
  auto multitracks = H5::VarLenType(&multitrack);

  // now define the main `Entry` structure
  H5::CompType entryType(sizeof(Entry));
  // entryType.insertMember("value_d", simple_offset(&Entry::value_d), dtype);
  // entryType.insertMember("value_i", simple_offset(&Entry::value_i), itype);
  // Note that for variable length types we need to use special containers
  // each of these has an `h5` member which HDF5 can recognize.
  // Since this is the first member of the classes the offset within the
  // class is technically zero, but better to point to it explicitly.
  // entryType.insertMember("value_s", offset(&Entry::value_s), stype);
  // entryType.insertMember("vector_d", offset(&Entry::vector_d), vl_dtype);
  // entryType.insertMember("vector_s", offset(&Entry::vector_s), vl_stype);
  // entryType.insertMember("vv_i", offset(&Entry::vv_i), vvl_itype);
  // entryType.insertMember("tracks", offset(&Entry::tracks), tracksType);
  entryType.insertMember("verts", offset(&Entry::verts), multitracks);
  return entryType;
}


int main(int argc, char* argv[]) {

  // _______________________________________________________________________
  // Part 3: setup outputs

  // Build the output file. Since multiple writes happen throughout
  // the run we have to build this _before_ looping through entries.
  //
  // I'm using HDF_ACC_TRUNC here, which tells HDF5 to overwrite
  // existing files. It's safer to use H5F_ACC_EXCL, which will throw
  // an exception rather than overwrite.
  H5::H5File file("test.h5", H5F_ACC_TRUNC);

  // The output buffer is periodically flushed.
  const size_t buffer_size = 100;

  // The more complicated buffer stores entries of type `entryType`
  // from above.
  auto entryType = getEntryType();
  OneDimBuffer<Entry> ebuffer(file, "entries", entryType, buffer_size);

  // _______________________________________________________________________
  // Part 4: generate some bogus data

  for (int iii = 0; iii < 500; iii++) {

    // Create an `Entry` object and fill it with garbage data.
    Entry entry;
    for (int jjj = 0; jjj < (iii % 10); jjj++) {
      MultiTrack tk;
      for (int kkk = 0; kkk < jjj; kkk++) {
	tk.tracks.push_back({100.0*jjj, std::sin(jjj)});
	tk.n_tracks = jjj;
      }
      entry.verts.push_back(tk);
    }

    // add the `Entry` to the buffer.
    ebuffer.push_back(entry);
  }
  // Buffers need to be flushed after the loop, since `flush` is only
  // called automatically when the buffer fills up.
  ebuffer.flush();
  return 0;
}
