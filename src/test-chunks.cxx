#include "tools.hh"

#include <vector>
#include <iostream>
#include "H5Cpp.h"

constexpr const hsize_t operator""_ht (unsigned long long int iii) {
  return static_cast<const hsize_t>(iii);
}

int main(int argc, char* argv[]) {
  hsize_t initial[1] = {0};
  hsize_t eventual[1] = {H5S_UNLIMITED};

  H5::DataSpace orig_space(1, initial, eventual);

  H5::DSetCreatPropList params;
  hsize_t chunk_size[1] = {100};
  params.setChunk(1, chunk_size);

  H5::H5File file("test.h5", H5F_ACC_TRUNC);
  auto dtype = H5::PredType::NATIVE_INT;
  H5::DataSet dataset = file.createDataSet(
    "the_data", dtype, orig_space, params);

  hsize_t max_size = 15;
  std::vector<int> buffer;
  hsize_t offset = 0;
  OneDimBuffer<int> other_buf(file, "other_data", dtype);
  for (int iii = 0; iii < 100; iii++) {
    std::cout << "pushing " << iii << std::endl;
    buffer.push_back(iii);
    other_buf.push_back(iii * 4);
    if (buffer.size() == max_size) {
      hsize_t slab_dims[1] = {buffer.size()};
      hsize_t total_dims[1] = {slab_dims[0] + offset};
      dataset.extend(total_dims);
      H5::DataSpace file_space = dataset.getSpace();
      H5::DataSpace mem_space(1, slab_dims);
      hsize_t offset_dims[1] = {offset};
      std::cout << "selecting" << std::endl;
      file_space.selectHyperslab(H5S_SELECT_SET, slab_dims, offset_dims);
      std::cout << "writing" << std::endl;
      dataset.write(buffer.data(), dtype, mem_space, file_space);
      offset += buffer.size();
      buffer.clear();
    }
  }
}
