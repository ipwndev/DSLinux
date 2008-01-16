#serial 11
dnl Copyright (C) 2005, 2006 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_FTS],
[
  AC_REQUIRE([gl_FUNC_FTS_CORE])
])

AC_DEFUN([gl_FUNC_FTS_LGPL],
[
  AC_REQUIRE([gl_FUNC_FTS_CORE])
  AC_DEFINE([_LGPL_PACKAGE], 1,
    [Define to 1 if compiling for a package to be distributed under the
     GNU Lesser Public License.])
])

AC_DEFUN([gl_FUNC_FTS_CORE],
[
  dnl Use this version of fts unconditionally, since the GNU libc and
  dnl NetBSD versions have bugs and/or unnecessary limitations.
  AC_LIBOBJ([fts])

  dnl Prerequisites of lib/fts.c.
  AC_REQUIRE([gl_FUNC_OPENAT])

  # Checks for header files.
  AC_CHECK_HEADERS_ONCE([sys/param.h])dnl
])
