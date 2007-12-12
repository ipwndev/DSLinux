# This file is generated automatically. Do not edit!
# Makefile for GNU tar library.	-*- Makefile -*-

# Copyright (C) 1994, 1995, 1996, 1997, 1999, 2000, 2001, 2003, 2004,
# 2005, 2006 Free Software Foundation, Inc.

## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.

## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.

## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
## 02110-1301, USA.

noinst_LIBRARIES = libtar.a
noinst_HEADERS = system.h system-ioctl.h localedir.h rmt.h paxlib.h stdopen.h
libtar_a_SOURCES = prepargs.c prepargs.h rtapelib.c paxerror.c paxexit.c paxnames.c stdopen.c

localedir = $(datadir)/locale

DISTCLEANFILES = localedir.h
localedir.h : Makefile
	echo '#define LOCALEDIR "$(localedir)"' >$@
	echo "#ifndef DEFAULT_RMT_COMMAND" >> $@
	echo "# define DEFAULT_RMT_COMMAND \"$(DEFAULT_RMT_DIR)/`echo rmt | sed '$(transform)'`$(EXEEXT)\"" >> $@
	echo "#endif" >> $@

rtapelib.o: localedir.h

libtar_a_LIBADD = $(LIBOBJS) $(ALLOCA)
libtar_a_DEPENDENCIES = $(libtar_a_LIBADD)

BUILT_SOURCES =
AM_CPPFLAGS =
EXTRA_DIST = Makefile.tmpl
MAINTAINERCLEANFILES =
MOSTLYCLEANFILES =
lib_OBJECTS = $(libtar_a_OBJECTS)

# Special rule for getdate
#
# Say $(srcdir), so GNU make does not report an ambiguity with the .y.c rule.
$(srcdir)/getdate.c: getdate.y
	cd $(srcdir) && \
	  $(YACC) $(YFLAGS) getdate.y && \
	  mv -f y.tab.c getdate.c

SUFFIXES = .o .c .h
CLEANFILES =
# gnulib modules
# alloca

# alloca-opt
BUILT_SOURCES += $(ALLOCA_H)
EXTRA_DIST += alloca_.h

# We need the following in order to create <alloca.h> when the system
# doesn't have one that works with the given compiler.
alloca.h: alloca_.h
	cp -f $(srcdir)/alloca_.h $@-t
	mv -f $@-t $@
MOSTLYCLEANFILES += alloca.h alloca.h-t

# allocsa
libtar_a_SOURCES += allocsa.h allocsa.c
EXTRA_DIST += allocsa.valgrind

# argmatch
EXTRA_DIST += argmatch.h

# argp
libtar_a_SOURCES += argp.h argp-ba.c argp-eexst.c \
           argp-fmtstream.c argp-fmtstream.h argp-fs-xinl.c argp-help.c \
           argp-namefrob.h argp-parse.c argp-pin.c argp-pv.c argp-pvh.c \
           argp-xinl.c

# backupfile
EXTRA_DIST += backupfile.h

# chdir-long
EXTRA_DIST += chdir-long.h

# chown

# clock-time

# close-stream
EXTRA_DIST += close-stream.h

# closeout
EXTRA_DIST += closeout.h

# configmake
# Retrieve values of the variables through 'configure' followed by
# 'make', not directly through 'configure', so that a user who
# sets some of these variables consistently on the 'make' command
# line gets correct results.
#
# One advantage of this approach, compared to the classical
# approach of adding -DLIBDIR=\"$(libdir)\" etc. to AM_CPPFLAGS,
# is that it protects against the use of undefined variables.
# If, say, $(libdir) is not set in the Makefile, LIBDIR is not
# defined by this module, and code using LIBDIR gives a
# compilation error.
#
# Another advantage is that 'make' output is shorter.
#
# Listed in the same order as the GNU makefile conventions.
# The Automake-defined pkg* macros are appended, in the order
# listed in the Automake documentation.
configmake.h: Makefile
	rm -f $@-t $@
	( \
	  echo '#define PREFIX "$(prefix)"'; \
	  echo '#define EXEC_PREFIX "$(exec_prefix)"'; \
	  echo '#define BINDIR "$(bindir)"'; \
	  echo '#define SBINDIR "$(sbindir)"'; \
	  echo '#define LIBEXECDIR "$(libexecdir)"'; \
	  echo '#define DATAROOTDIR "$(datarootdir)"'; \
	  echo '#define DATADIR "$(datadir)"'; \
	  echo '#define SYSCONFDIR "$(sysconfdir)"'; \
	  echo '#define SHAREDSTATEDIR "$(sharedstatedir)"'; \
	  echo '#define LOCALSTATEDIR "$(localstatedir)"'; \
	  echo '#define INCLUDEDIR "$(includedir)"'; \
	  echo '#define OLDINCLUDEDIR "$(oldincludedir)"'; \
	  echo '#define DOCDIR "$(docdir)"'; \
	  echo '#define INFODIR "$(infodir)"'; \
	  echo '#define HTMLDIR "$(htmldir)"'; \
	  echo '#define DVIDIR "$(dvidir)"'; \
	  echo '#define PDFDIR "$(pdfdir)"'; \
	  echo '#define PSDIR "$(psdir)"'; \
	  echo '#define LIBDIR "$(libdir)"'; \
	  echo '#define LISPDIR "$(lispdir)"'; \
	  echo '#define LOCALEDIR "$(localedir)"'; \
	  echo '#define MANDIR "$(mandir)"'; \
	  echo '#define MANEXT "$(manext)"'; \
	  echo '#define PKGLIBDIR "$(pkglibdir)"'; \
	  echo '#define PKGINCLUDEDIR "$(pkgincludedir)"'; \
	  echo '#define PKGDATADIR "$(pkgdatadir)"'; \
	:) | sed '/""/d' >$@-t
	mv $@-t $@
BUILT_SOURCES += configmake.h
CLEANFILES += configmake.h configmake.h-t

# d-ino

# dirname
EXTRA_DIST += dirname.h

# error

# exclude
EXTRA_DIST += exclude.h

# exit
libtar_a_SOURCES += exit.h

# exitfail
EXTRA_DIST += exitfail.h

# extensions

# fcntl
BUILT_SOURCES += $(FCNTL_H)
EXTRA_DIST += fcntl_.h

# We need the following in order to create <fcntl.h> when the system
# doesn't have one that works with the given compiler.
fcntl.h: fcntl_.h
	rm -f $@-t $@
	sed -e 's|@''ABSOLUTE_FCNTL_H''@|$(ABSOLUTE_FCNTL_H)|g' \
	    < $(srcdir)/fcntl_.h > $@-t
	mv $@-t $@
MOSTLYCLEANFILES += fcntl.h fcntl.h-t

# fcntl-safer
EXTRA_DIST += fcntl-safer.h fcntl--.h

# fileblocks

# fnmatch
BUILT_SOURCES += $(FNMATCH_H)
EXTRA_DIST += fnmatch_.h fnmatch_loop.c

# We need the following in order to create <fnmatch.h> when the system
# doesn't have one that supports the required API.
fnmatch.h: fnmatch_.h
	cp -f $(srcdir)/fnmatch_.h $@-t
	mv -f $@-t $@
MOSTLYCLEANFILES += fnmatch.h fnmatch.h-t

# fnmatch-gnu

# fpending
EXTRA_DIST += __fpending.h

# ftruncate

# full-write
libtar_a_SOURCES += full-write.h full-write.c

# getcwd
EXTRA_DIST += getcwd.h

# getdate
libtar_a_SOURCES += getdate.y
BUILT_SOURCES += getdate.c
MAINTAINERCLEANFILES += getdate.c
EXTRA_DIST += getdate.c getdate.h

# getdelim
EXTRA_DIST += getdelim.h

# getline
EXTRA_DIST += getline.h

# getopt
BUILT_SOURCES += $(GETOPT_H)
EXTRA_DIST += getopt_.h getopt_int.h

# We need the following in order to create <getopt.h> when the system
# doesn't have one that works with the given compiler.
getopt.h: getopt_.h
	cp -f $(srcdir)/getopt_.h $@-t
	mv -f $@-t $@
MOSTLYCLEANFILES += getopt.h getopt.h-t

# getpagesize
EXTRA_DIST += getpagesize.h

# gettext
# This is for those projects which use "gettextize --intl" to put a source-code
# copy of libintl into their package. In such projects, every Makefile.am needs
# -I$(top_builddir)/intl, so that <libintl.h> can be found in this directory.
# For the Makefile.ams in other directories it is the maintainer's
# responsibility; for the one from gnulib we do it here.
# This option has no effect when the user disables NLS (because then the intl
# directory contains no libintl.h file) or when the project does not use
# "gettextize --intl".
AM_CPPFLAGS += -I$(top_builddir)/intl

# gettext-h
libtar_a_SOURCES += gettext.h

# gettime

# gettimeofday

# hash
EXTRA_DIST += hash.h

# havelib

# human
EXTRA_DIST += human.h

# intprops
EXTRA_DIST += intprops.h

# inttostr
EXTRA_DIST += inttostr.c inttostr.h

# inttypes
BUILT_SOURCES += $(INTTYPES_H)
EXTRA_DIST += inttypes_.h

# We need the following in order to create <inttypes.h> when the system
# doesn't have one that works with the given compiler.
inttypes.h: inttypes_.h
	rm -f $@-t $@
	sed -e 's/@''HAVE_INTTYPES_H''@/$(HAVE_INTTYPES_H)/g' \
	    -e 's|@''ABSOLUTE_INTTYPES_H''@|$(ABSOLUTE_INTTYPES_H)|g' \
	    -e 's/@''PRI_MACROS_BROKEN''@/$(PRI_MACROS_BROKEN)/g' \
	    -e 's/@''HAVE_LONG_LONG_INT''@/$(HAVE_LONG_LONG_INT)/g' \
	    -e 's/@''PRIPTR_PREFIX''@/$(PRIPTR_PREFIX)/g' \
	    -e 's/@''HAVE_DECL_IMAXABS''@/$(HAVE_DECL_IMAXABS)/g' \
	    -e 's/@''HAVE_DECL_IMAXDIV''@/$(HAVE_DECL_IMAXDIV)/g' \
	    -e 's/@''HAVE_DECL_STRTOIMAX''@/$(HAVE_DECL_STRTOIMAX)/g' \
	    -e 's/@''HAVE_DECL_STRTOUMAX''@/$(HAVE_DECL_STRTOUMAX)/g' \
	    < $(srcdir)/inttypes_.h > $@-t
	mv $@-t $@
MOSTLYCLEANFILES += inttypes.h inttypes.h-t

# lchown
EXTRA_DIST += lchown.h

# localcharset
libtar_a_SOURCES += localcharset.h localcharset.c
EXTRA_DIST += config.charset ref-add.sin ref-del.sin

# We need the following in order to install a simple file in $(libdir)
# which is shared with other installed packages. We use a list of referencing
# packages so that "make uninstall" will remove the file if and only if it
# is not used by another installed package.
# On systems with glibc-2.1 or newer, the file is redundant, therefore we
# avoid installing it.

all-local: charset.alias ref-add.sed ref-del.sed

charset_alias = $(DESTDIR)$(libdir)/charset.alias
charset_tmp = $(DESTDIR)$(libdir)/charset.tmp
install-exec-local: all-local
	test $(GLIBC21) != no || $(mkinstalldirs) $(DESTDIR)$(libdir)
	if test -f $(charset_alias); then \
	  sed -f ref-add.sed $(charset_alias) > $(charset_tmp) ; \
	  $(INSTALL_DATA) $(charset_tmp) $(charset_alias) ; \
	  rm -f $(charset_tmp) ; \
	else \
	  if test $(GLIBC21) = no; then \
	    sed -f ref-add.sed charset.alias > $(charset_tmp) ; \
	    $(INSTALL_DATA) $(charset_tmp) $(charset_alias) ; \
	    rm -f $(charset_tmp) ; \
	  fi ; \
	fi

uninstall-local: all-local
	if test -f $(charset_alias); then \
	  sed -f ref-del.sed $(charset_alias) > $(charset_tmp); \
	  if grep '^# Packages using this file: $$' $(charset_tmp) \
	      > /dev/null; then \
	    rm -f $(charset_alias); \
	  else \
	    $(INSTALL_DATA) $(charset_tmp) $(charset_alias); \
	  fi; \
	  rm -f $(charset_tmp); \
	fi

charset.alias: config.charset
	rm -f t-$@ $@
	$(SHELL) $(srcdir)/config.charset '$(host)' > t-$@
	mv t-$@ $@

SUFFIXES += .sed .sin
.sin.sed:
	rm -f t-$@ $@
	sed -e '/^#/d' -e 's/@''PACKAGE''@/$(PACKAGE)/g' $< > t-$@
	mv t-$@ $@

CLEANFILES += charset.alias ref-add.sed ref-del.sed

# lstat
EXTRA_DIST += lstat.h

# malloc

# mbchar
libtar_a_SOURCES += mbchar.h

# mbuiter
libtar_a_SOURCES += mbuiter.h

# memchr

# mempcpy
EXTRA_DIST += mempcpy.h

# memrchr
EXTRA_DIST += memrchr.h

# memset

# minmax
libtar_a_SOURCES += minmax.h

# mkdtemp
libtar_a_SOURCES += mkdtemp.h

# mktime

# modechange
EXTRA_DIST += modechange.h

# obstack

# openat
EXTRA_DIST += at-func.c openat.h openat-priv.h

# quote
EXTRA_DIST += quote.h

# quotearg
EXTRA_DIST += quotearg.h

# regex
EXTRA_DIST += regcomp.c regex.h regex_internal.c regex_internal.h regexec.c

# rmdir

# rpmatch

# safe-read
EXTRA_DIST += safe-read.h

# safe-write
EXTRA_DIST += safe-write.h

# save-cwd
EXTRA_DIST += save-cwd.h

# savedir
EXTRA_DIST += savedir.h

# setenv
libtar_a_SOURCES += setenv.h

# size_max
libtar_a_SOURCES += size_max.h

# ssize_t

# stat-macros
EXTRA_DIST += stat-macros.h

# stat-time
EXTRA_DIST += stat-time.h

# stdarg

# stdbool
BUILT_SOURCES += $(STDBOOL_H)
EXTRA_DIST += stdbool_.h

# We need the following in order to create <stdbool.h> when the system
# doesn't have one that works.
stdbool.h: stdbool_.h
	rm -f $@-t $@
	sed -e 's/@''HAVE__BOOL''@/$(HAVE__BOOL)/g' < $(srcdir)/stdbool_.h > $@-t
	mv $@-t $@
MOSTLYCLEANFILES += stdbool.h stdbool.h-t

# stdint
BUILT_SOURCES += $(STDINT_H)
EXTRA_DIST += stdint_.h

# We need the following in order to create <stdint.h> when the system
# doesn't have one that works with the given compiler.
stdint.h: stdint_.h
	rm -f $@-t $@
	sed -e 's/@''HAVE_WCHAR_H''@/$(HAVE_WCHAR_H)/g' \
	    -e 's/@''HAVE_STDINT_H''@/$(HAVE_STDINT_H)/g' \
	    -e 's|@''ABSOLUTE_STDINT_H''@|$(ABSOLUTE_STDINT_H)|g' \
	    -e 's/@''HAVE_SYS_TYPES_H''@/$(HAVE_SYS_TYPES_H)/g' \
	    -e 's/@''HAVE_INTTYPES_H''@/$(HAVE_INTTYPES_H)/g' \
	    -e 's/@''HAVE_SYS_INTTYPES_H''@/$(HAVE_SYS_INTTYPES_H)/g' \
	    -e 's/@''HAVE_SYS_BITYPES_H''@/$(HAVE_SYS_BITYPES_H)/g' \
	    -e 's/@''HAVE_LONG_LONG_INT''@/$(HAVE_LONG_LONG_INT)/g' \
	    -e 's/@''BITSIZEOF_PTRDIFF_T''@/$(BITSIZEOF_PTRDIFF_T)/g' \
	    -e 's/@''PTRDIFF_T_SUFFIX''@/$(PTRDIFF_T_SUFFIX)/g' \
	    -e 's/@''BITSIZEOF_SIG_ATOMIC_T''@/$(BITSIZEOF_SIG_ATOMIC_T)/g' \
	    -e 's/@''HAVE_SIGNED_SIG_ATOMIC_T''@/$(HAVE_SIGNED_SIG_ATOMIC_T)/g' \
	    -e 's/@''SIG_ATOMIC_T_SUFFIX''@/$(SIG_ATOMIC_T_SUFFIX)/g' \
	    -e 's/@''BITSIZEOF_SIZE_T''@/$(BITSIZEOF_SIZE_T)/g' \
	    -e 's/@''SIZE_T_SUFFIX''@/$(SIZE_T_SUFFIX)/g' \
	    -e 's/@''BITSIZEOF_WCHAR_T''@/$(BITSIZEOF_WCHAR_T)/g' \
	    -e 's/@''HAVE_SIGNED_WCHAR_T''@/$(HAVE_SIGNED_WCHAR_T)/g' \
	    -e 's/@''WCHAR_T_SUFFIX''@/$(WCHAR_T_SUFFIX)/g' \
	    -e 's/@''BITSIZEOF_WINT_T''@/$(BITSIZEOF_WINT_T)/g' \
	    -e 's/@''HAVE_SIGNED_WINT_T''@/$(HAVE_SIGNED_WINT_T)/g' \
	    -e 's/@''WINT_T_SUFFIX''@/$(WINT_T_SUFFIX)/g' \
	    < $(srcdir)/stdint_.h > $@-t
	mv $@-t $@
MOSTLYCLEANFILES += stdint.h stdint.h-t

# stpcpy
libtar_a_SOURCES += stpcpy.h

# strcase
libtar_a_SOURCES += strcase.h

# strchrnul
libtar_a_SOURCES += strchrnul.h

# strdup
EXTRA_DIST += strdup.h

# strndup
EXTRA_DIST += strndup.h

# strnlen
EXTRA_DIST += strnlen.h

# strnlen1
libtar_a_SOURCES += strnlen1.h strnlen1.c

# strtoimax

# strtol

# strtoll

# strtoul

# strtoull

# strtoumax

# sysexits
BUILT_SOURCES += $(SYSEXITS_H)
EXTRA_DIST += sysexit_.h

# We need the following in order to create <sysexits.h> when the system
# doesn't have one that works with the given compiler.
sysexits.h: sysexit_.h
	cp -f $(srcdir)/sysexit_.h $@-t
	mv -f $@-t $@
MOSTLYCLEANFILES += sysexits.h sysexits.h-t

# time_r
EXTRA_DIST += time_r.h

# timespec
EXTRA_DIST += timespec.h

# unistd
BUILT_SOURCES += $(UNISTD_H)

# We need the following in order to create an empty placeholder for
# <unistd.h> when the system doesn't have one.
unistd.h:
	echo '/* Empty placeholder for $@.  */' >$@
MOSTLYCLEANFILES += unistd.h

# unistd-safer
EXTRA_DIST += unistd-safer.h unistd--.h

# unlinkdir
EXTRA_DIST += unlinkdir.h

# unlocked-io
EXTRA_DIST += unlocked-io.h

# utime

# utimens
EXTRA_DIST += utimens.h

# vasnprintf
libtar_a_SOURCES += printf-args.h printf-parse.h vasnprintf.h

# verify
libtar_a_SOURCES += verify.h

# version-etc
libtar_a_SOURCES += version-etc.h version-etc.c

# version-etc-fsf
libtar_a_SOURCES += version-etc-fsf.c

# vsnprintf
libtar_a_SOURCES += vsnprintf.h

# wcwidth
libtar_a_SOURCES += wcwidth.h

# xalloc
EXTRA_DIST += xalloc.h

# xalloc-die
libtar_a_SOURCES += xalloc-die.c

# xgetcwd
EXTRA_DIST += xgetcwd.h

# xsize
libtar_a_SOURCES += xsize.h

# xstrndup
libtar_a_SOURCES += xstrndup.h xstrndup.c

# xstrtol
EXTRA_DIST += xstrtol.h

# xstrtoumax
libtar_a_SOURCES += xstrtoumax.c

