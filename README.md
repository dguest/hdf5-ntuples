HDF5 Examples
-------------

# What is this for?

This is a few tests assess the usefulness of a lightweight C++ based
root to HDF5 converter. The goals are to:

 - Reduce dependencies. This works without `PyRoot`, `RootPy`, or any
   other fancy stuff. The dependencies are currently:

    + A C++11 compiler (tested with Clang, should work with gcc)
    + HDF5 with C++ bindings.

   Obviously we'll eventually need a working version of ROOT.

 - Keep the interface relatively close to the HDF5 bindings.

# What does it do right now?

Not a lot. There are a few routines that write out dummy ntuples.
If this turns out to be useful it may be developed more.

# Quickstart

 1. Clone this repository
 2. Run `make`
 3. Run `./bin/test-chunks` to produce the file `test.h5`
 4. Use `h5ls -d test.h5/entries` to dump the data you just wrote.
