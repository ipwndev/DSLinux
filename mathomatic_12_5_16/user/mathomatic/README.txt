                                   Mathomatic
                                       by
                               George Gesslein II

This archive contains the C source code for Mathomatic, the automatic algebraic
manipulator.  It should compile and run correctly under Unix, GNU/Linux, Mac
OS-X, and CygWin, without any modifications.

Mathomatic is a portable, general purpose CAS (Computer Algebra System) that
can automatically solve, simplify, combine, and compare algebraic equations,
perform complex and polynomial arithmetic, etc.  It does some calculus and is
very easy to use.

To compile, just type "make" at a shell prompt.  This will compile the C source
code to create the executable file named "mathomatic".  To run Mathomatic, type
"./mathomatic" at the shell prompt.

To test most functionality, type "make test".

To install, you have to be the super-user, then type "make install".

To add "readline" editing and history of all Mathomatic input, type:

	make clean
	make READLINE=1

This allows you to use the cursor keys to recall and edit previous expressions.

A typical installation is done by typing the following at the shell prompt:

	make READLINE=1
	make test
	sudo make install

To uninstall from the system, type:

	sudo make uninstall

To compile the secure version, with no file I/O or shelling out, type:

	./compile.secure

This will silently create the executable "mathomatic_secure", which can safely
be used as a telnet application or CGI program.

This software is copyrighted and made available under the GNU Lesser GPL (see
file "COPYING").

There are quite a few math goodies in the source archive, besides the main
Mathomatic program:

  The file "changes.txt" is the version history of Mathomatic.
  The directory "doc" contains the HTML Mathomatic documentation.
  The directory "tests" contains test scripts and other interesting scripts.
  The directory "primes" contains the Mathomatic Prime Number Tools.
  The directory "lib" contains the hooks and test for the Mathomatic library.
  The directory "fact" contains factorial functions in various languages.

For quick help while running Mathomatic, use the "help" command.  To read the
documentation, point your web browser to the "doc" directory.

For the latest source code and information, go to the Mathomatic website:

	http://www.mathomatic.org

Author e-mail: gesslein@panix.com

Author postal address:

	George Gesslein II
	875 Cobb Street
	Groton, New York 13073
	USA

The file "tasks.txt" contains a number of good ideas to implement, when someone
is kind enough to allow me to do something.


             Compile-Time Defines for the Mathomatic Source Code
             ---------------------------------------------------

To compile Mathomatic for UNIX, GNU/Linux, Mac OS-X, or any POSIX compliant OS,
define "UNIX".  To compile Mathomatic for a generic system, simply compile with
no defines.  To compile for Microsoft Win32 using CygWin, define "CYGWIN".

Define "READLINE" and include the readline libraries at link time to use
readline mode.  This will allow easy command line editing and history.

Define "BASICS" to remove some commands (nintegrate and code generation) that
aren't required for basic functionality.  This is useful for creating a
stripped down version.  Code size is reduced with this option.

Define "SILENT" to remove all helpful messages and debugging code.  This is
useful when using Mathomatic as a symbolic math library.  Code size is reduced
with this option.

Define "LIBRARY" when using as a symbolic math library.  "SILENT" is
automatically defined when this is defined.  All standard input and output is
disabled and the code will function as a library.  See the directory "lib" for
the library hooks and test.  Some commands are omitted with this option.

Define "SECURE" to disable file reading and writing and shell escape.  This is
useful when making Mathomatic available to the public through telnet or CGI
programs.  It is also useful when making ROMable code.  Code size is reduced
with this option.  Some insecure commands are omitted with this option.

Define "TIMEOUT_SECONDS" to set the maximum number of seconds Mathomatic may
run.  Upon timeout, Mathomatic properly exits.  This is useful when making
Mathomatic a CGI program, so it won't overload the server.

Define "I18N" to enable internationalization.
