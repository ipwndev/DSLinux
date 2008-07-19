# sys_stat_h.m4 serial 4   -*- Autoconf -*-
dnl Copyright (C) 2006 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl From Eric Blake.
dnl Test whether <sys/stat.h> contains lstat and mkdir or must be substituted.

AC_DEFUN([gl_HEADER_SYS_STAT_H],
[
  dnl Check for lstat.  Systems that lack it (mingw) also lack symlinks, so
  dnl stat is a good replacement.
  AC_CHECK_FUNCS_ONCE([lstat])

  dnl Check for mkdir.  Mingw has _mkdir(name) in the nonstandard <io.h>
  dnl instead.
  AC_CHECK_DECLS([mkdir],
    [],
    [AC_CHECK_HEADERS([io.h])],
    [#include <sys/stat.h>])
  AC_REQUIRE([AC_C_INLINE])

  dnl Check for broken stat macros.
  AC_REQUIRE([AC_HEADER_STAT])

  gl_ABSOLUTE_HEADER([sys/stat.h])
  ABSOLUTE_SYS_STAT_H=\"$gl_cv_absolute_sys_stat_h\"
  AC_SUBST([ABSOLUTE_SYS_STAT_H])
  SYS_STAT_H='sys/stat.h'
  AC_SUBST([SYS_STAT_H])
]) # gl_HEADER_SYS_STAT_H
