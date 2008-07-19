dnl Curses detection: Taken from giFTcurs acinclude.m4

dnl What it does:
dnl =============
dnl
dnl - Determine which version of curses is installed on your system
dnl   and set the -I/-L/-l compiler entries and add a few preprocessor
dnl   symbols 
dnl - Do an AC_SUBST on the CURSES_INCLUDEDIR and CURSES_LIBS so that
dnl   @CURSES_INCLUDEDIR@ and @CURSES_LIBS@ will be available in
dnl   Makefile.in's
dnl - Modify the following configure variables (these are the only
dnl   curses.m4 variables you can access from within configure.in)
dnl   CURSES_INCLUDEDIR - contains -I's and possibly -DRENAMED_CURSES if
dnl                       an ncurses.h that's been renamed to curses.h
dnl                       is found.
dnl   CURSES_LIBS       - sets -L and -l's appropriately
dnl   CFLAGS            - if --with-sco, add -D_SVID3 
dnl   has_curses        - exports result of tests to rest of configure
dnl
dnl Usage:
dnl ======
dnl 1) call AC_CHECK_CURSES after AC_PROG_CC in your configure.in
dnl 2) Instead of #include <curses.h> you should use the following to
dnl    properly locate ncurses or curses header file
dnl
dnl    #if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
dnl    #include <ncurses.h>
dnl    #else
dnl    #include <curses.h>
dnl    #endif
dnl
dnl 3) Make sure to add @CURSES_INCLUDEDIR@ to your preprocessor flags
dnl 4) Make sure to add @CURSES_LIBS@ to your linker flags or LIBS
dnl
dnl Notes with automake:
dnl - call AM_CONDITIONAL(HAS_CURSES, test "$has_curses" = true) from
dnl   configure.in
dnl - your Makefile.am can look something like this
dnl   -----------------------------------------------
dnl   INCLUDES= blah blah blah $(CURSES_INCLUDEDIR) 
dnl   if HAS_CURSES
dnl   CURSES_TARGETS=name_of_curses_prog
dnl   endif
dnl   bin_PROGRAMS = other_programs $(CURSES_TARGETS)
dnl   other_programs_SOURCES = blah blah blah
dnl   name_of_curses_prog_SOURCES = blah blah blah
dnl   other_programs_LDADD = blah
dnl   name_of_curses_prog_LDADD = blah $(CURSES_LIBS)
dnl   -----------------------------------------------


AC_DEFUN([AC_CHECK_CURSES],[
	search_ncurses=true
	screen_manager=""
	has_curses=false
	has_wide_curses=no

	CFLAGS=${CFLAGS--O}

	AC_SUBST(CURSES_LIBS)
	AC_SUBST(CURSES_INCLUDEDIR)

	AC_ARG_WITH(sunos-curses,
	  [  --with-sunos-curses     used to force SunOS 4.x curses],[
	  if test x$withval = xyes; then
		AC_USE_SUNOS_CURSES
	  fi
	])

	AC_ARG_WITH(osf1-curses,
	  [  --with-osf1-curses      used to force OSF/1 curses],[
	  if test x$withval = xyes; then
		AC_USE_OSF1_CURSES
	  fi
	])

	AC_ARG_WITH(vcurses,
	  [[  --with-vcurses[=incdir] used to force SysV curses]],
	  if test x$withval != xyes; then
		CURSES_INCLUDEDIR="-I$withval"
	  fi
	  AC_USE_SYSV_CURSES
	)

	AC_ARG_WITH(ncurses,
	  [[  --with-ncurses[=dir]    compile with ncurses/locate base dir]],
	  if test x$withval = xno ; then
		search_ncurses=false
	  elif test x$withval != xyes ; then
		CURSES_LIBS="$LIBS -L$withval/lib -lncurses"
		CURSES_INCLUDEDIR="-I$withval/include"
		if test -f $withval/include/curses.h
		then
			CURSES_INCLUDEDIR="$CURSES_INCLUDEDIR -DRENAMED_NCURSES"
		fi
		search_ncurses=false
		screen_manager="ncurses"
		AC_DEFINE(USE_NCURSES, 1, [Use Ncurses?])
		AC_DEFINE(HAS_CURSES, 1, [Found some version of curses that we're going to use])
		has_curses=true
	  fi
	)

	AC_ARG_WITH(ncursesw,
	  [[  --with-ncursesw[=dir]   compile with ncursesw/locate base dir]],
	  if test x$withval = xyes; then
		AC_NCURSES(/usr/include/ncursesw, curses.h, -lncursesw, -I/usr/include/ncursesw -DRENAMED_NCURSES, renamed ncursesw on /usr/include/ncursesw)
	    search_ncurses=false
	  elif test x$withval != xyes ; then
		CURSES_LIBS="$LIBS -L$withval/lib -lncursesw"
		CURSES_INCLUDEDIR="-I$withval/include"
		if test -f $withval/include/curses.h
		then
			CURSES_INCLUDEDIR="$CURSES_INCLUDEDIR -DRENAMED_NCURSES"
		fi
	    search_ncurses=false
		screen_manager="ncursesw"
		AC_DEFINE(USE_NCURSES, 1)
		AC_DEFINE(HAS_CURSES, 1)
		has_curses=true
	  fi
	)

	if $search_ncurses
	then
		AC_SEARCH_NCURSES()
	fi

	dnl Check for some functions
	SAVED_LIBS="$LIBS"
	LIBS="$CURSES_LIBS"
	unset ac_cv_func_wadd_wch
	AC_CHECK_FUNCS(use_default_colors resizeterm resize_term wadd_wch)
	LIBS="$SAVED_LIBS"

	dnl See if it's a wide curses
	if test $ac_cv_func_wadd_wch = yes; then
		has_wide_curses=yes
		AH_VERBATIM([_XOPEN_SOURCE_EXTENDED],
		[/* Enable X/Open Unix extensions */
#ifndef _XOPEN_SOURCE_EXTENDED
# define _XOPEN_SOURCE_EXTENDED
#endif])
		AC_DEFINE(WIDE_NCURSES, 1, [curses routines to work with wide chars are available])
	fi
])


AC_DEFUN([AC_USE_SUNOS_CURSES], [
	search_ncurses=false
	screen_manager="SunOS 4.x /usr/5include curses"
	AC_MSG_RESULT(Using SunOS 4.x /usr/5include curses)
	AC_DEFINE(USE_SUNOS_CURSES, 1, [Use SunOS SysV curses?])
	AC_DEFINE(HAS_CURSES, 1)
	has_curses=true
	AC_DEFINE(NO_COLOR_CURSES, 1, [If your curses does not have color define this one])
	AC_DEFINE(USE_SYSV_CURSES, 1, [Use SystemV curses?])
	CURSES_INCLUDEDIR="-I/usr/5include"
	CURSES_LIBS="/usr/5lib/libcurses.a /usr/5lib/libtermcap.a"
	AC_MSG_RESULT(Please note that some screen refreshes may fail)
])

AC_DEFUN([AC_USE_OSF1_CURSES], [
       AC_MSG_RESULT(Using OSF1 curses)
       search_ncurses=false
       screen_manager="OSF1 curses"
       AC_DEFINE(HAS_CURSES, 1)
       has_curses=true
       AC_DEFINE(NO_COLOR_CURSES, 1)
       AC_DEFINE(USE_SYSV_CURSES, 1)
       CURSES_LIBS="-lcurses"
])

AC_DEFUN([AC_USE_SYSV_CURSES], [
	AC_MSG_RESULT(Using SysV curses)
	AC_DEFINE(HAS_CURSES, 1)
	has_curses=true
	AC_DEFINE(USE_SYSV_CURSES)
	search_ncurses=false
	screen_manager="SysV/curses"
	CURSES_LIBS="-lcurses"
])

dnl
dnl Parameters: directory filename curses_LIBS curses_INCLUDEDIR nicename
dnl
AC_DEFUN([AC_NCURSES], [
    if $search_ncurses
    then
        if test -f $1/$2
	then
	    AC_MSG_RESULT(Found ncurses on $1/$2)
 	    CURSES_LIBS="$3"
	    CURSES_INCLUDEDIR="$4"
	    search_ncurses=false
	    screen_manager="$5"
            AC_DEFINE(HAS_CURSES, 1)
            has_curses=true
	    AC_DEFINE(USE_NCURSES, 1)
	fi
    fi
])

AC_DEFUN([AC_SEARCH_NCURSES], [
    AC_CHECKING(location of ncurses.h file)

    AC_NCURSES(/usr/include, ncurses.h, -lncurses,, ncurses on /usr/include)
    AC_NCURSES(/usr/include/ncurses, ncurses.h, -lncurses, -I/usr/include/ncurses, ncurses on /usr/include/ncurses)
    AC_NCURSES(/usr/local/include, ncurses.h, -L/usr/local/lib -lncurses, -I/usr/local/include, ncurses on /usr/local)
    AC_NCURSES(/usr/local/include/ncurses, ncurses.h, -L/usr/local/lib -L/usr/local/lib/ncurses -lncurses, -I/usr/local/include/ncurses, ncurses on /usr/local/include/ncurses)

    dnl ncurses hides here on OS X
    AC_NCURSES(/sw/include, ncurses.h, -L/sw/lib -lncurses, -I/sw/include, ncurses on /sw/include)

    AC_NCURSES(/usr/local/include/ncurses, curses.h, -L/usr/local/lib -lncurses, -I/usr/local/include/ncurses -DRENAMED_NCURSES, renamed ncurses on /usr/local/.../ncurses)

    AC_NCURSES(/usr/include/ncurses, curses.h, -lncurses, -I/usr/include/ncurses -DRENAMED_NCURSES, renamed ncurses on /usr/include/ncurses)

    dnl
    dnl We couldn't find ncurses, try SysV curses
    dnl
    if $search_ncurses 
    then
        AC_EGREP_HEADER(init_color, /usr/include/curses.h,
	    AC_USE_SYSV_CURSES)
	AC_EGREP_CPP(USE_NCURSES,[
#include <curses.h>
#ifdef __NCURSES_H
#undef USE_NCURSES
USE_NCURSES
#endif
],[
	CURSES_INCLUDEDIR="$CURSES_INCLUDEDIR -DRENAMED_NCURSES"
        AC_DEFINE(HAS_CURSES, 1)
	has_curses=true
        AC_DEFINE(USE_NCURSES, 1)
        search_ncurses=false
        screen_manager="ncurses installed as curses"
])
    fi

    dnl
    dnl Try SunOS 4.x /usr/5{lib,include} ncurses
    dnl The flags USE_SUNOS_CURSES, USE_BSD_CURSES and BUGGY_CURSES
    dnl should be replaced by a more fine grained selection routine
    dnl
	if $search_ncurses; then
		if test -f /usr/5include/curses.h
		then
			AC_USE_SUNOS_CURSES
		fi
	fi
])

dnl Check if gcc accepts the -no-cpp-precomp flag. (Mac OS X thingee)
dnl AC_NO_CPP_PRECOMP
AC_DEFUN(AC_NO_CPP_PRECOMP,
[
	AC_CACHE_CHECK([if $CC needs -no-cpp-precomp],
				   [ac_no_cpp_precomp],
				   [echo "void f(){}" > conftest.c
					if test -z "`${CC} -no-cpp-precomp -c conftest.c 2>&1`"; then
						ac_no_cpp_precomp=yes
					else
						ac_no_cpp_precomp=no
					fi
					rm -f conftest*
				   ])
	if test "x$ac_no_cpp_precomp" = "xyes"; then
		CFLAGS="$CFLAGS -no-cpp-precomp"
	fi
])

dnl Check what flags gcc accepts and add them to CFLAGS
dnl AX_TRY_GCC_FLAGS([flags])
dnl Written by Go"ran Weinholt.
AC_DEFUN(AX_TRY_GCC_FLAGS,
[
	if test "x$GCC" = xyes; then
		AC_CACHE_CHECK([for flags to pass to gcc], ax_cv_try_gcc_flags,
					   [echo "void f(void){}" > conftest.c
						ax_cv_try_gcc_flags=
						for flag in $1; do
							if test -z "`${CC} $flag -c conftest.c 2>&1`"; then
								ax_cv_try_gcc_flags="$flag $ax_cv_try_gcc_flags"
							fi
						done
						rm -f conftest.c conftest.$ac_ext
					   ])
		if test -n "$ax_cv_try_gcc_flags"; then
			CFLAGS="$CFLAGS $ax_cv_try_gcc_flags"
		fi
	fi
])
