# -*- makefile -*-

AUTOMAKE_OPTIONS = foreign
INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = chvt deallocvt writevt fgconsole screendump setvesablank \
	vcstime vt-is-UTF8 openvt @RESIZECONS@
EXTRA_PROGRAMS = resizecons

LDADD = ../lib/libctlocal.a ../lib/libconsole.la ../lib/libctutils.la

vcstime_LDADD = ../lib/libctlocal.a
screendump_LDADD = ../lib/libctlocal.a ../lib/libctutils.la
writevt_LDADD = ../lib/libctlocal.a ../lib/libctutils.la
resizecons_LDADD =  ../lib/libcfont.la $(LDADD)
