
This build tree consists of the following:

 * incl1, incl2: include directories that contain header files.  This
   is not a model of how to organize header files as it violates some
   of the best practices in the documentation, but we set it up this
   way for the verification suite to exercise the ccxx logic of moving
   header files from one location to another.  Ensuring that this
   works properly makes sure that the compiler support's gen_deps is
   working as it should.  (For example, removing -MP from gcc's
   invocation would cause thsi to fail for gcc.)

 * a, x: static library build items

 * b: shared library build item

 * src: build item consisting of static library Y, shared library Z,
   and executable main.  Shared library Z calls static library Y.

We verify several things:

 * The intial builds everything, and the program runs properly with
   the two shared libraries being found.

 * When we remove a source file from the list of sources that compile,
   all libraries get rebuild and all shared libaries and programs get
   relinked; the program runs without the code from the removed object
   file

 * When we move a header, everything that depends on the header and
   everything that links with things that depend on the header
   (executables and shared libaries but not static libraries)
   rebuilds.

 * A build when everything is up to date doesn't build anything.  This
   helps to make sure that the detection of orphan targets is not
   getting false positives.
