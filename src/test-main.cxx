#include "tools.hh"
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>

#include "H5Cpp.h"


struct Point {
  double thed;
  int thei;
  const char* thes;
};



int main(int argc, char* argv[]) {

  H5::H5File out_file("test.h5", H5F_ACC_EXCL);

  hsize_t nentry = 4;
  std::vector<Point> stuff;
  std::vector<std::unique_pointer<const char*>> strings;
  strings.reserve(20);
  for (int iii = 0; iii < nentry; iii++) {
    double fval = iii;
    strings.push_back(std::make_unique<const char*>(std::to_string));
    stuff.push_back(Point{fval, iii, strings.back().c_str()});
  }
  hsize_t dim[] = {nentry};
  H5::DataSpace space(1, dim);

  auto stype = H5::StrType(H5::PredType::C_S1, H5T_VARIABLE);
  stype.setCset(H5T_CSET_UTF8);

  H5::CompType type(sizeof(Point));
  type.insertMember("adub", HOFFSET(Point, thed),
		    H5::PredType::NATIVE_DOUBLE);
  type.insertMember("anint", HOFFSET(Point, thei),
		    H5::PredType::NATIVE_INT);
  type.insertMember("thestr", HOFFSET(Point, thes),
		    stype);

  auto dtype = H5::PredType::NATIVE_DOUBLE;
  std::vector<double> some_date(nentry, 9.0);
  H5::DataSet dataset(
    out_file.createDataSet("the_ds", dtype, space));
  dataset.write(some_date.data(), dtype);
  auto attr = dataset.createAttribute("theatr", type, space);
  attr.write(type, stuff.data());


  return 0;
}
