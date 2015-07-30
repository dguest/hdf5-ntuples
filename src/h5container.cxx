#include "h5container.hh"

// #include <iostream>

namespace h5 {
  // ______________________________________________________________________
  // implementation for string
  string::string(): _string()
  {
    reset();
  }

  string::string(const string& old): _string(old._string)
  {
    reset();
  }
  string::string(const std::string& old): _string(old)
  {
    reset();
  }
  string::string(string&& old): _string(std::move(old._string))
  {
    reset();
  }
  string& string::operator=(string old)
  {
    std::swap(this->_string, old._string);
    reset();
    return *this;
  }

  size_t string::size() const {return _string.size(); }
  const char* string::data() const {return _string.data(); }

  void string::reset() {
    h5 = _string.data();
  }
  // end of string

  // _______________________________________________________________________
  // specializations
  void reset(vector<string>& cont) {
    cont._char_vector.clear();
    for (auto& entry: cont._vector) {
      cont._char_vector.push_back(entry.data());
    }
    cont.h5.p = cont._char_vector.data();
    cont.h5.len = cont._char_vector.size();
  }

}
