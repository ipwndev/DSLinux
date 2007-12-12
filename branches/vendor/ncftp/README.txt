0.  For the attention-span challenged
-------------------------------------

Want filename completion?  It's built-in, GNU Readline isn't necessary.
Want full-screen support?  Install Ncurses if your OS's curses stinks.
NcFTP uses "passive" by default.  Try "set passive off" if needed.
Yes, it's Y2K compliant.


1.  But what is this?
---------------------

NcFTP is a free set of programs that use the File Transfer Protocol.  It's
been around for quite some time now (circa 1992) and many people still use
the program and contribute suggestions and feedback.

The main program is simply called "ncftp".  There are also separate utility
programs for one-shot FTP operations (i.e. for shell scripts and command
line junkies);  these include "ncftpget", "ncftpput", and "ncftpls".  Run
each command without any arguments to see the usage screen, or read the
man page.

Also included is a batch processing daemon, "ncftpbatch", which
is invoked by the "bgget" command from "ncftp" and also the "-b" flag of
"ncftpput" and "ncftpget".  Along with "ncftpbatch", which is what we
call a "personal FTP spooler", there is also "ncftpspooler" which can be
used as a system-wide FTP spooler.

Lastly, the "ncftpbookmarks" program is a full-screen utility program to
manipulate user's FTP bookmarks.  Currently this is the only program which
uses full-screen mode.


2. General Instructions
-----------------------

To build NcFTP and the utility programs, you can simply do the following:

    1.  (Optional) Install ncurses (available from a GNU mirror).
    2.  Run the "./configure" script in the NcFTP source directory.
    3.  (Probably not necessary) Inspect the Makefiles in each subdirectory.
    4.  (Probably not necessary) Browse and edit config.h.
    5.  "make"
    6.  "make install"


3. Curses (Full-screen mode)
----------------------------

You may find that the full-screen portions of NcFTP don't compile or don't
work to your satisfaction.  The official gospel on this topic is to install
the Ncurses library.  Things have improved in the past few years, but many
major vendor's implementations of curses (the full-screen library) are still
broken or leave much to be desired.  Still, even using ncurses or a good
version of curses in a program is far from bullet-proof, which is why this
version of NcFTP uses less of it.

You should have the version of curses you want NcFTP to use installed before
you "./configure" NcFTP.  If you didn't do that, you should remove the
"config.cache" file in NcFTP's source directory and reconfigure so it looks
for the new libraries.

If all else fails, you can remove the "config.cache" file and
"./configure --disable-curses".  You'll probably need to do this if you
find that ncftpbookmarks won't compile.  (Or you can just use the binaries
that did build in the "bin" directory.)


4. OEM installations using shared libraries
-------------------------------------------

If you're daring, and want to build the best ncftp configuration, you can
build the LibNcFTP (the FTP library which the programs use) as a shared
library.

To do that, after running configure, go to the libncftp subdirectory and
build libncftp.so.  The Makefile shows how to do this using the GNU
development tools, so for example under linux you would just need to
"make shared" and then install the resulting libncftp.so.3 in your system's
shared library directory (with "make soinstall").


6.  Contact information
-----------------------

Mike Gleason
NcFTP Software
mgleason@NcFTP.com
http://www.NcFTP.com
