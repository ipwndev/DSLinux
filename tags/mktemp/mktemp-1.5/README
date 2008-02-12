What/why is mktemp?
===================
Mktemp is a simple utility designed to make temporary file handling
in shells scripts be safe and simple.  Traditionally, people writing
shell scripts have used constructs like:

    TFILE=/tmp/foop.$$

which are trivial to attack.  If such a script is run as root it may
be possible for an attacker on the local host to gain access to the
root login, corrupt or unlink system files, or do a variety of other
nasty things.

The basic problem is that most shells have no equivalent to open(2)'s
O_EXCL flag.  While it is possible to avoid this using temporary
directories, I consider the use of mktemp(1) to be superior both in terms
of simplicity and robustness.  See the man page for more information.

I originally wrote mktemp(1) for the OpenBSD operating system and
this version tracks any changes made to the mktemp(1) included with
OpenBSD.  Subsequently, many of the major Linux distributions started
to include it in their distributions.  I strongly encourage other
OS vendors to either include mktemp(1) or something like it with
their base OS.

Where to get it
===============
The latest version of mktemp may always be gotten via anonymous ftp
from ftp.mktemp.org in the directory /pub/mktemp/.  You can also
find it on the web at http://www.mktemp.org/dist/.

Copyright
=========
Mktemp is distributed under a BSD-style license.  Please refer to
the `LICENSE' file included with the release for details.  If you
are an OS vendor who would like to bundle mktemp(1) but its license
is unacceptable to you, please contact me--we can probably work
something out.

Web page
========
There is a mktemp `home page' at http://www.mktemp.org/
that contains on-line documentation and other information.

Mailing lists
=============
The mktemp-announce mailing list receives new release announcements and
information about mktemp-related security issues.

To subscribe, please visit the following web page:
    http://www.mktemp.org/mailman/listinfo/mktemp-announce

Bug reports
===========
If you find a bug in mktemp, please use the bug database on the web
at http://www.mktemp.org/bugs/.
