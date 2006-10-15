# configure.ac - process this file with autoconf to produce configure
# Copyright (C) 2002, 2003, 2004, 2005, 2006 Arthur de Jong
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

AC_PREREQ(2.60a)
AC_COPYRIGHT(
[Copyright (C) 2002, 2003, 2004, 2005, 2006 Arthur de Jong

This configure script is derived from configure.ac which is free software;
you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.
See the configure.ac file for more details.])

# initialize and set version and bugreport address
AC_INIT([rl],[0.2.5],[arthur@ch.tudelft.nl])
RELEASE_MONTH="Sep 2006"
AC_SUBST(RELEASE_MONTH)
AC_CONFIG_SRCDIR(rl.c)

# display notice and initialize automake
AC_MSG_NOTICE([configuring AC_PACKAGE_TARNAME AC_PACKAGE_VERSION])
AM_INIT_AUTOMAKE(AC_PACKAGE_TARNAME,AC_PACKAGE_VERSION)

# create a config.h file (Automake will add -DHAVE_CONFIG_H)
AC_CONFIG_HEADERS(config.h)

# AJ_CHECK_VALUE(decl, [includes])
# my cusom test (based on AC_CHECK_SIZEOF)
AC_DEFUN([AJ_CHECK_VALUE],
         [AS_LITERAL_IF([$1], [],
                        [AC_FATAL([$0: requires literal arguments])])
             AC_COMPUTE_INT([value of $1],[aj_cv_value_$1],
                            [(long)($1)],[AC_INCLUDES_DEFAULT([$2])],
                            [AC_MSG_FAILURE([cannot compute value ($1), 77])])
         ])# AJ_CHECK_VALUE

# checks for programs
AC_PROG_INSTALL
AC_PROG_CC

# checks for header files
AC_HEADER_STDC
AC_CHECK_HEADERS(getopt.h)
AC_CHECK_HEADERS(string.h)
AC_CHECK_HEADERS(errno.h)
AC_CHECK_HEADERS(stdio.h)
AC_CHECK_HEADERS(sys/time.h)
AC_CHECK_HEADERS(limits.h)
AC_CHECK_HEADERS(libintl.h)
AC_HEADER_TIME

# checks for library functions
AC_CHECK_FUNCS([gettimeofday strdup strerror strtol malloc realloc])
AC_REPLACE_FUNCS(getopt_long)

# do some value checks
AC_TYPE_SIZE_T
AC_CHECK_SIZEOF(int)
AC_CHECK_DECL(INT_MAX,,,[#include <limits.h>])
AJ_CHECK_VALUE(INT_MAX,[#include <limits.h>])
AC_CHECK_DECL(RAND_MAX)
AJ_CHECK_VALUE(RAND_MAX)
AC_C_CONST
AC_HEADER_TIME

# GCC compiler options
AC_C_INLINE
if test x$CC = xgcc; then
  CFLAGS="$CFLAGS -Wall"
fi

# check for options
AC_ARG_ENABLE(debug,
              AS_HELP_STRING(--enable-debug,enable extensive debugging to stderr),
              CFLAGS="-DDEBUG $CFLAGS")

# check for extra compiler warnings
AC_ARG_ENABLE(warnings,
              AS_HELP_STRING(--enable-warnings,enable extra compiler warnings (gcc only)),
              [if test "x$enableval" != "no" ; then CFLAGS="$CFLAGS -pedantic -Wall -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Waggregate-return -Wmissing-declarations" ; fi])

# check which random number generator to use
WHICH_RNG="rand"
AC_ARG_WITH(rng,
            AS_HELP_STRING([--with-rng@<:@=NAME@:>@],
                           [choose rng (rand,dev)]),
            [WHICH_RNG="$withval"])
case "$WHICH_RNG" in
rand)
  AC_MSG_NOTICE([using system rand() function])
  AC_DEFINE_UNQUOTED(USE_RAND_RNG,1,[use standard rand() rng])
  ;;
dev)
  AC_MSG_NOTICE([using system /dev/urandom random device])
  AC_DEFINE_UNQUOTED(USE_DEV_RNG,1,[use /dev/urandom rng])
  ;;
*)
  AC_MSG_ERROR([*** illegal value specified for  --with-rng ***])
  ;;
esac

# what files to output
AC_CONFIG_FILES(Makefile rl.lsm rl.spec rl.1)
AC_OUTPUT
