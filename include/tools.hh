#ifndef TOOLS_HH
#define TOOLS_HH

#include "H5Cpp.h"
#include <string>
#include <vector>

template<typename T>
class OneDimBuffer
{
public:
  OneDimBuffer(H5::CommonFG& group, std::string ds_name,
	       H5::DataType type, hsize_t = 10);
  void push_back(T new_entry);
  void flush();
private:
  H5::DataType _type;
  hsize_t _max_size;
  hsize_t _offset;
  std::vector<T> _buffer;
  H5::DataSet _ds;
};

template<typename T>
OneDimBuffer<T>::OneDimBuffer(
  H5::CommonFG& group, std::string ds_name,
  H5::DataType type, hsize_t size):
  _type(type),
  _max_size(size),
  _offset(0)
{
  hsize_t initial[1] = {0};
  hsize_t eventual[1] = {H5S_UNLIMITED};

  H5::DataSpace orig_space(1, initial, eventual);

  H5::DSetCreatPropList params;
  hsize_t chunk_size[1] = {100};
  params.setChunk(1, chunk_size);

  _ds = group.createDataSet(ds_name, _type, orig_space, params);
}

template<typename T>
void OneDimBuffer<T>::push_back(T new_entry) {
  if (_buffer.size() == _max_size) {
    flush();
  }
  _buffer.push_back(new_entry);
}
template<typename T>
void OneDimBuffer<T>::flush() {
  if (_buffer.size() == 0) return;
  hsize_t slab_dims[1] = {_buffer.size()};
  hsize_t total_dims[1] = {slab_dims[0] + _offset};
  _ds.extend(total_dims);
  H5::DataSpace file_space = _ds.getSpace();
  H5::DataSpace mem_space(1, slab_dims);
  hsize_t offset_dims[1] = {_offset};
  // std::cout << "selecting" << std::endl;
  file_space.selectHyperslab(H5S_SELECT_SET, slab_dims, offset_dims);
  // std::cout << "writing" << std::endl;
  _ds.write(_buffer.data(), _type, mem_space, file_space);
  _offset += _buffer.size();
  _buffer.clear();
}

#endif
