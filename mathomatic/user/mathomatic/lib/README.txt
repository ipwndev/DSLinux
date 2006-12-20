This directory contains the hooks and test for the Mathomatic symbolic math
library.  Compile and link "*.c" and "../*.c", leaving out "../main.c", with
"-DLIBRARY" on the C compiler command line, to create the test app which uses
the library.

See the file "lib.c" for the very simple API, and "test.c" for an example on
how to use it.

Note that "equation-number-range" is disabled in commands in the library.  This
is because only one equation result may be returned at a time.  A single
equation number is allowed for the "equation-number-range" argument.

Also note that the "set display2d" option doesn't do anything in the library,
the returned result is always a single text line, except when redirecting
command output to a file.
