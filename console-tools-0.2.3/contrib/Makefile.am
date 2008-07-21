# -*- makefile -*-

AUTOMAKE_OPTIONS = foreign
INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = codepage splitfont

bin_SCRIPTS = fix_bs_and_del font2psf
LDADD = ../lib/libctlocal.a ../lib/libctutils.la
noinst_SCRIPTS = cad

EXTRA_DIST = README.cad \
	$(bin_SCRIPTS) $(noinst_SCRIPTS)
