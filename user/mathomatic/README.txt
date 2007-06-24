                                   Mathomatic
                                       by
                               George Gesslein II

This archive contains the C source code and documentation for Mathomatic, the
automatic algebraic manipulator.  It should compile and run correctly under
Unix, GNU/Linux, Mac OS-X, and CygWin for MS-Windows, without any
modifications.

Mathomatic is a portable, general purpose CAS (Computer Algebra System) that
can symbolically solve, simplify, combine, and compare algebraic equations,
perform complex number and polynomial arithmetic, etc.  It does some calculus
and is very easy to use.

All software and documentation in this archive is copyrighted and made
available under the GNU Lesser GPL version 2.1 (see file "COPYING").

                                  Compilation
                                  -----------

To compile Mathomatic without readline support, just type "make" at a shell
prompt.  This will compile the C source code to create the executable file
named "mathomatic".  To run Mathomatic, type "./mathomatic" at the shell
prompt.

To test most functionality, type:

	make test

To recompile with readline editing and history of all Mathomatic input, type:

	make clean
	make READLINE=1

This allows you to use the cursor keys to recall and edit previous expressions.

                                  Installation
                                  ------------

A typical installation is done by typing the following at the shell prompt:

	make READLINE=1
	make test
	sudo make install

This will compile, test, and install the Mathomatic executable and docs in
"/usr/local" in less than a minute.

To completely remove Mathomatic from the system, type:

	sudo make uninstall

To compile the secure version, with no file I/O or shelling out, type:

	./compile.secure

This will silently create the executable "mathomatic_secure", which can safely
be used as a telnet application or CGI program.

There are quite a few math goodies in this archive, besides the main Mathomatic
program:

  The directory "doc" contains the Mathomatic documentation in HTML.
  The directory "tests" contains test scripts and other interesting scripts.
  The directory "primes" contains the Mathomatic Prime Number Tools.
  The directory "lib" contains the hooks and test for the Mathomatic library.
  The directory "factorial" contains factorial functions in various languages.
  The file "complex_lib.c" is a generic, floating point complex number library.

For quick help while running Mathomatic, use the "help" command.  To read the
documentation, point your web browser to the file "doc/index.html" or
"/usr/local/share/doc/mathomatic/html/index.html" if you ran "make install".
When copying the Mathomatic documentation, please copy the entire documentation
directory, and not selected files from it.

For the latest source code, documentation, and information, go to the
Mathomatic website: http://www.mathomatic.org

Author email: gesslein@panix.com

Please consider sending the author an email.  Any email with the subject
"mathomatic" will make it through the spam filter.

Don't forget to donate to the author if you make money off of Mathomatic.

Author postal address:

	George Gesslein II
	875 Cobb Street
	Groton, New York 13073
	USA

Please report any bugs you find to the author.  Thanks go to Jochen Plumeyer
for hosting the Mathomatic web site.


             Compile-Time Defines for the Mathomatic Source Code
             ---------------------------------------------------

To compile Mathomatic for UNIX, GNU/Linux, Mac OS-X, or any POSIX compliant OS,
define "UNIX" (see "makefile").  To compile Mathomatic for a generic system,
simply compile with no defines.  To compile for Microsoft Win32 using CygWin,
define "CYGWIN" (see "makefile.cygwin").

Define "READLINE" and include the readline libraries at link time to use
readline mode.  This will allow easy command line editing and history.

Define "SILENT" to remove all helpful messages and debugging code.  This is
useful when using Mathomatic as a symbolic math library.  Code size is reduced
with this option.

Define "LIBRARY" when using as a symbolic math library.  "SILENT" is
automatically defined when this is defined.  Most standard input and output is
disabled and the code will function as a library.  See the directory "lib" and
the file "makefile.lib" for the library hooks and test.  The following commands
are omitted with this option: calculate, code, divide, edit, nintegrate,
optimize, pause, quit, roots, and tally.

Define "SECURE" to disable file reading and writing and shell escape.  This is
useful when making Mathomatic available to the public through telnet or CGI
programs.  It is also useful when making ROMable code.  All insecure commands
are omitted with this option.  See "compile.secure", which is the secure
Mathomatic build script.

Define "TIMEOUT_SECONDS" to set the maximum number of seconds Mathomatic may
run.  Upon timeout, Mathomatic properly exits.  This is useful when making
Mathomatic a CGI program, so it won't overload the server.

Define "I18N" to enable internationalization.


                         Mathomatic C source code files
                         ------------------------------

  am.h - the main include file for Mathomatic, contains tunable parameters
  complex.h - floating point complex number arithmetic function prototypes
  externs.h - global variable extern definitions
  proto.h - global function prototypes

  am.c - many necessary routines
  cmds.c - code for commands that don't belong anywhere else
  complex.c - floating point complex number routines
  complex_lib.c - general floating point complex number arithmetic library
  diff.c - differentiation routines and commands
  factor.c - symbolic factorizing routines
  factor_int.c - floating point constant factorizing routines
  gcd.c - floating point Greatest Common Divisor code
  globals.c - global variable and array definitions
  help.c - command table, help command code, and parsing routines
  integrate.c - integration routines and commands
  list.c - expression and equation display routines
  main.c - startup code for Mathomatic
  parse.c - expression parsing routines
  poly.c - simplifying and polynomial routines
  simplify.c - simplifying routines
  solve.c - solve routines
  super.c - group and combine denominators
  unfactor.c - unfactorizing (expanding) routines
