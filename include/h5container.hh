// Container classes
// TODO:
//  - Make sure the h5::vector constructor works on nested std::vector.
//  - Figure out how to avoid calling `reset()` on each push_back call,
//    we only really need this when the vector is moved after we exceed
//    capacity.
//  - Make a `getType` method which returns the HDF5 datatype.

#ifndef H5_CONTAINER_HH
#define H5_CONTAINER_HH

#include <vector>
#include "H5Cpp.h"

// #include <iostream>

namespace h5 {
  template<typename T>
  class vector
  {
  public:
    vector();
    vector(const vector&);
    vector(const std::vector<T>&);
    vector(vector&&);
    vector(std::vector<T>&&);
    vector& operator=(vector);
    void push_back(T value);
    size_t size() const;
    T* data();
    hvl_t h5;

    // The internals have to be public to keep the compiler from
    // complaining about non-standard layout, but the data below
    // should be considered private. At the very least, be careful
    // modifying these members (in particular, call `reset()` if you
    // do anything that could invalidate their pointers).
    void reset();
    std::vector<T> _vector;
    std::vector<hvl_t> _hvl_vector;
    // specialization for strings
    std::vector<const char*> _char_vector;
  };

  // string class
  class string
  {
  public:
    string();
    string(const string&);
    string(const std::string&);
    string(string&&);
    string& operator=(string);
    size_t size() const;
    const char* data() const;
    const char* h5;

    // see warning above wrt calling `reset()` and the internal container
    void reset();
    std::string _string;
  };

  // ______________________________________________________________________
  // implementation for vector

  // define global functions to help with nested container specialization
  void reset(vector<string>& );
  template<typename T>
  void reset(vector<vector<T>>& );
  template<typename T>
  void reset(vector<T>& );

  // boilerplate constructor, copy constructor, etc
  // anything that modifies the vector must reset the pointers
  template<typename T>
  vector<T>::vector(): _vector()
  {
    reset();
  }
  template<typename T>
  vector<T>::vector(const vector& old): _vector(old._vector)
  {
    reset();
  }
  template<typename T>
  vector<T>::vector(const std::vector<T>& old_vec): _vector(old_vec)
  {
    reset();
  }
  template<typename T>
  vector<T>::vector(vector&& old):
    _vector(std::move(old._vector))
  {
    reset();
  }
  template<typename T>
  vector<T>::vector(std::vector<T>&& old):
    _vector(std::move(old))
  {
    reset();
  }
  template<typename T>
  vector<T>& vector<T>::operator=(vector old)
  {
    std::swap(this->_vector, old._vector);
    reset();
    return *this;
  }

  // several basic access functions
  template<typename T>
  void vector<T>::push_back(T value)
  {
    _vector.push_back(value);
  }
  template<typename T>
  size_t vector<T>::size() const { return _vector.size(); }
  template<typename T>
  T* vector<T>::data() { return _vector.data(); }

  // the `reset` calls above call the global reset function
  // which makes template specialization easier (at least for me)
  template<typename T>
  void vector<T>::reset() {
    // this just calls the global function
    // specializations are below
    h5::reset(*this);
  }

  // global functions for vector reset
  // nested vector case
  template<typename T>
  void reset(vector<vector<T> >& cont) {
    cont._hvl_vector.clear();
    for (auto& entry: cont._vector) {
      hvl_t hvl_entry;
      hvl_entry.len = entry.size();
      hvl_entry.p = entry.data();
      cont._hvl_vector.push_back(hvl_entry);
    }
    cont.h5.p = cont._hvl_vector.data();
    cont.h5.len = cont._hvl_vector.size();
  }

  // the generic class case
  template<typename T>
  void reset(vector<T>& cont) {
    cont.h5.p = cont._vector.data();
    cont.h5.len = cont._vector.size();
  }

}
#endif
