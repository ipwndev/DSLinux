#serial 9
dnl Copyright (C) 2002, 2005, 2006 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FOPEN_SAFER],
[
  AC_LIBOBJ([fopen-safer])
  AC_DEFINE([GNULIB_FOPEN_SAFER], [1],
    [Define to 1 when using the gnulib fopen-safer module.])
])

AC_DEFUN([gl_TMPFILE_SAFER],
[
  AC_LIBOBJ([tmpfile-safer])
])
