This directory contains the hooks and test for the Mathomatic symbolic math
library.  Compile and link "*.c" and "../*.c" with "-DLIBRARY" on the C
compiler command line, to create the test app which uses the library.  This is
done by "../makefile.lib".  To compile the library and test, type:

	cd ..
	make clean
	make -f makefile.lib

This will create the library test executable "mathomatic" and the library
file "libmathomatic.a".  To install the library in "/usr/lib", type:

	sudo make -f makefile.lib install

See the file "lib.c" for the very simple API, and "test.c" for an example on
how to use it.  Just include the file "mathomatic.h" and call the functions in
"lib.c" to use the library.

Note that "equation-number-range" is disabled in commands in the library.  This
is because only one equation result may be returned at a time.  A single
equation number is allowed for the "equation-number-range" argument.
