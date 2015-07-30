HDF5 Examples
-------------

### What is this for? ###

This is a viability test for a lightweight C++ based ROOT to HDF5
converter. The goals are to:

 - Reduce dependencies. We want this to work without `PyRoot`,
   `RootPy`, or any of the other awesome libraries that never seem to
   compile on the server you want them to work on. The dependencies
   are currently:

	+ A C++11 compiler (tested with Clang and gcc)
	+ HDF5 with C++ bindings.

   Obviously we'll eventually need a working version of ROOT.

 - Support writing of D3PD-like data structures, i.e. arbitrary-depth
   nested vectors containing arbitrary data types.

 - Have (relatively) high performance. Entries are buffered before writing
   them to file to reduce the number of write operations.

 - Keep the interface relatively close to the HDF5 bindings. We're not
   trying to abstract _everything_ away.

### What does it do right now? ###

Not a lot. There are a few routines that write out dummy ntuples.
If this turns out to be useful it may be developed more.

### Quickstart ###

 1. Clone [this repository][repo]
 2. Run `make`
 3. Run `./bin/test-chunks` to produce the file `test.h5`
 4. Use `h5ls -d test.h5/entries` to dump the data you just wrote.

[repo]: https://github.com/dguest/hdf5-ntuples "github repo"

### Awesome, so how do I make it do something useful? ###

The `test-chunks` routine serves as a (fairly exhaustive) example of
how to write out arbitrary data. The source file for this executable
is `src/test-chunks.cxx`, hopefully the comments there will explain
the internals in more detail.

The basic idea is to:

 1. Define an output structure (i.e. a `struct`)
 2. Define output types as `H5::DataType` objects.
 3. Setup the outputs with `H5::H5File` and `OneDimBuffer`.
 4. Write out some dummy data.

The makefile will automatically compile anything beginning with
`src/test-*` as an executable in `bin/`, so you can simply copy any
files beginning with `test-*` and hack away.
