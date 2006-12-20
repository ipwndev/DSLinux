# Makefile.am - use automake to generate Makefile.in
# Copyright (C) 2001, 2002, 2003, 2005, 2006 Arthur de Jong
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

bin_PROGRAMS=rl
rl_SOURCES=rl.c rl.h \
           alloc.c alloc.h \
           buffer.c buffer.h \
           random.c random.h \
           aclocal.m4
rl_LDADD=@LIBOBJS@

EXTRA_DIST=rl.lsm.in rl.spec.in getopt_long.c getopt_long.h \
           debian/control debian/changelog debian/rules debian/docs \
           debian/copyright debian/compat

#  install the man pages
man_MANS=rl.1

# targets for debian package
deb: distdir
	cd $(distdir) && \
	  fakeroot debian/rules binary
	rm -rf $(distdir)
signeddeb: distdir
	cd $(distdir) && \
	  debuild
	rm -rf $(distdir)

changelog:
	svn2cl -i
