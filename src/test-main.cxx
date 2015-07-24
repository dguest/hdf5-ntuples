#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>

#include "H5Cpp.h"


struct Point {
  double thed;
  int thei;
  const char* thes;
  hvl_t ints;
  hvl_t int_arrays;
};



int main(int argc, char* argv[]) {

  H5::H5File out_file("test.h5", H5F_ACC_EXCL);

  hsize_t nentry = 4;
  std::vector<Point> stuff;
  std::vector<std::string> strings;
  strings.reserve(20);
  std::vector<int*> the_ints;
  the_ints.reserve(20);
  std::vector<std::vector<int*> > the_int_arrays;
  the_int_arrays.reserve(20);
  for (int iii = 0; iii < nentry; iii++) {
    double fval = iii;
    strings.push_back(std::to_string(iii));
    the_ints.push_back(new int[iii]);
    the_int_arrays.push_back(std::vector<int*>());
    for (int jjj = 0; jjj < iii; jjj++) {
      the_ints.back()[jjj] = iii + jjj;
      the_int_arrays.back().push_back(new int[4]);
      for (int kkk = 0; kkk < 4; kkk++) {
	the_int_arrays.back().back()[kkk] = 1;
      }
    }
    stuff.push_back(Point{fval, iii, strings.back().c_str()});
    stuff.back().ints.p = the_ints.back();
    stuff.back().ints.len = iii;
    stuff.back().int_arrays.len = iii;
    hvl_t* the_subarray = new hvl_t[iii];
    for (int jjj = 0; jjj < iii; jjj++) {
      the_subarray[jjj].p = the_int_arrays.at(iii)[jjj];
      the_subarray[jjj].len = 4;
    }
    stuff.back().int_arrays.p = the_subarray;
  }
  hsize_t dim[] = {nentry};
  H5::DataSpace space(1, dim);

  auto stype = H5::StrType(H5::PredType::C_S1, H5T_VARIABLE);
  stype.setCset(H5T_CSET_UTF8);
  auto itype = H5::DataType(H5::PredType::NATIVE_INT);
  auto vltype = H5::VarLenType(&itype);
  auto vvltype = H5::VarLenType(&vltype);

  H5::CompType type(sizeof(Point));
  type.insertMember("adub", HOFFSET(Point, thed),
		    H5::PredType::NATIVE_DOUBLE);
  type.insertMember("anint", HOFFSET(Point, thei),
		    H5::PredType::NATIVE_INT);
  type.insertMember("thestr", HOFFSET(Point, thes),
		    stype);
  type.insertMember("ints", HOFFSET(Point, ints),
		    vltype);
  type.insertMember("int_arrays", HOFFSET(Point, int_arrays),
		    vvltype);

  auto dtype = H5::PredType::NATIVE_DOUBLE;
  std::vector<double> some_date(nentry, 9.0);
  H5::DataSet dataset(
    out_file.createDataSet("the_ds", dtype, space));
  dataset.write(some_date.data(), dtype);
  auto attr = dataset.createAttribute("theatr", type, space);
  attr.write(type, stuff.data());


  return 0;
}
