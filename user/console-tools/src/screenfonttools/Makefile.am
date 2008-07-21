# -*- makefile -*-

AUTOMAKE_OPTIONS = foreign

ConsolecharsDefs = -DDATADIR=\"$(datadir)\" -DTRANSDIR=\"$(TRANSDIR)\"

INCLUDES = -I$(top_srcdir)/include $(ConsolecharsDefs)


bin_PROGRAMS = consolechars showcfont
LDADD = ../lib/libctlocal.a ../lib/libconsole.la ../lib/libctutils.la

consolechars_LDADD = ../lib/libctlocal.a ../lib/libconsole.la ../lib/libcfont.la ../lib/libctutils.la

bin_SCRIPTS = unicode_start unicode_stop charset

EXTRA_DIST = $(bin_SCRIPTS)

noinst_PROGRAMS = clrunimap
