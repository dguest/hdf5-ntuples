// Buffer for HDF5 objects, to handle writing buffered data to files.
//
// At the moment I'm still trying to figure out a strategy for
// variable-length types.

#ifndef ONE_DIM_BUFFER_HH
#define ONE_DIM_BUFFER_HH

#include "H5Cpp.h"
#include <string>
#include <vector>

namespace h5 {
  // Utility function to get a ``packed'' version of the datatype.
  // This lets us buffer structures which have stuff we don't want to write.
  H5::CompType packed(H5::CompType);
}

// _________________________________________________________________________
//                               public interface
//
// The buffer wraps a homogeneous list of objects with class `T` and
// supports only a few basic operations (like push_back).
template<typename T>
class OneDimBuffer
{
public:
  // Basic constructor.
  //  - The first argument should be a group or file,
  //  - the second is the name of this dataset within the file,
  //  - the third is the ``type'' as seen by HDF5.
  //  - The final entry is the max size of the buffer. It's only limited
  //    by your machine's memory.
  OneDimBuffer(H5::CommonFG& group, std::string ds_name,
	       H5::DataType type, hsize_t buffer_size = 10);

  // Constructor for compound types. We use this to make sure `pack`
  // is called on the on-disk datatype (so we don't save space for
  // bookkeeping info in the memory-resident objets).
  OneDimBuffer(H5::CommonFG& group, std::string ds_name,
	       H5::CompType type, hsize_t buffer_size = 10);

  // should be pretty self-explanatory...
  void push_back(T new_entry);
  // empty the buffer to disk
  void flush();
  // get the _total_ size (buffered and written)
  hsize_t size() const;

// ____________________________________________________________________
// implementation level stuff

private:
  // Internal constructor. Here we specify an in-memory and on-disk
  // datatype, since (in the case of compound datatypes) we don't
  // always want the same layout.
  OneDimBuffer(H5::CommonFG& group, std::string ds_name,
	       H5::DataType type, H5::DataType disk_type, hsize_t = 10);

  // In-memory datatype
  H5::DataType _type;

  // size where `flush()` is called
  hsize_t _max_size;

  // keep track of the current position in the disk dataset.
  hsize_t _offset;

  // buffer of objects in memory.
  std::vector<T> _buffer;

  // the dataset we're writing to.
  H5::DataSet _ds;
};

// Public constructors: thin wrappers on the internal constructor.
template<typename T>
OneDimBuffer<T>::OneDimBuffer(
  H5::CommonFG& group, std::string ds_name,
  H5::DataType type, hsize_t size):
  OneDimBuffer(group, ds_name, type, type, size)
{
}
template<typename T>
OneDimBuffer<T>::OneDimBuffer(
  H5::CommonFG& group, std::string ds_name,
  H5::CompType type, hsize_t size):
  OneDimBuffer(group, ds_name, type, h5::packed(type), size)
{
}

// the private constructor.
template<typename T>
OneDimBuffer<T>::OneDimBuffer(
  H5::CommonFG& group, std::string ds_name,
  H5::DataType type, H5::DataType disk_type, hsize_t buffer_size):
  _type(type),
  _max_size(buffer_size),
  _offset(0)
{
  // the dataset starts as a 1-dim array of zero length, but we can
  // expand it to infinity.
  hsize_t initial[1] = {0};
  hsize_t eventual[1] = {H5S_UNLIMITED};

  // the space occupied by the original dataset
  H5::DataSpace orig_space(1, initial, eventual);

  // We have to enable `chunking` in the file to save by block.  Not
  // sure what the optimum is, just go with buffer size for now.
  H5::DSetCreatPropList params;
  hsize_t chunk_size[1] = {buffer_size};
  params.setChunk(1, chunk_size);

  // Create the actual dataset.
  _ds = group.createDataSet(ds_name, disk_type, orig_space, params);
}

// Simple push_back function. Calls `flush()` if the buffer is full.
template<typename T>
void OneDimBuffer<T>::push_back(T new_entry) {
  if (_buffer.size() == _max_size) {
    flush();
  }
  _buffer.push_back(new_entry);
}

// This is where a lot of magic happens. We have to:
//  - Extend the dataset to accommodate the new entry.
//  - Select the part of the new dataset (the hyperslab) to write to.
//  - Write the buffer out.
//  - Update the offset.
template<typename T>
void OneDimBuffer<T>::flush() {
  if (_buffer.size() == 0) return;

  // The `hyperslab' we're writing to is the size of the buffer.
  hsize_t slab_dims[1] = {_buffer.size()};

  // The total size of the dataset must be extended to accomidate the
  // new entries.
  hsize_t total_dims[1] = {slab_dims[0] + _offset};
  _ds.extend(total_dims);

  // We'll only write to the end of the current file, so we select
  // the file's `DataSpace` and then pick only the entries corresponding
  // to the new slab we'll add.
  H5::DataSpace file_space = _ds.getSpace();
  H5::DataSpace mem_space(1, slab_dims);
  hsize_t offset_dims[1] = {_offset};
  file_space.selectHyperslab(H5S_SELECT_SET, slab_dims, offset_dims);

  // Write out data, update offset and clear buffer.
  _ds.write(_buffer.data(), _type, mem_space, file_space);
  _offset += _buffer.size();
  _buffer.clear();
}
template<typename T>
hsize_t OneDimBuffer<T>::size() const
{
  return _offset + _buffer.size();
}

// util functuinos
namespace h5 {
  H5::CompType packed(H5::CompType in) {
    // TODO: Figure out why a normal copy constructor doesn't work here.
    //       The normal one seems to create shallow copies.
    auto out = H5::CompType(H5Tcopy(in.getId()));
    out.pack();
    return out;
  }
}

#endif
