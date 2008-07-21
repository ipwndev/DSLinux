# -*- makefile -*-

AUTOMAKE_OPTIONS = foreign
INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = psfaddtable psfgettable psfstriptable

EXTRA_DIST = fonts2virfont.c virfont.h
# noinst_PROGRAMS = fonts2virfont

LDADD = ../lib/libctlocal.a ../lib/libcfont.la ../lib/libctutils.la

