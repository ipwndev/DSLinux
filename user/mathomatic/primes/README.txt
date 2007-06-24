This directory contains some small, integer math programs written in C.  To
compile and install, type:

	make
	make test
	sudo make install

This will install:

	matho-pascal - display Pascal's triangle
	matho-primes - generate consecutive prime numbers
	matho-sumsq - display minimum sum of the squares

Man pages are included.  These C programs are C++ compatible.

The Python program "primorial" is included for calculating large primorials
from "matho-primes".  To generate a list of all unique primorials from 2 to 97,
type the following:

	./primorial `matho-primes 2 97`
