# -*- makefile -*-

AUTOMAKE_OPTIONS = foreign

AnalyzeDefs = -DDATADIR=\"$(datadir)\" -DKEYMAPDIR=\"$(KEYMAPDIR)\"

INCLUDES = -I$(top_srcdir)/include $(AnalyzeDefs)


bin_PROGRAMS = @LOADKEYS@ dumpkeys showkey setleds setmetamode kbd_mode\
	@KEYCODES@

EXTRA_PROGRAMS = getkeycodes setkeycodes loadkeys

loadkeys_SOURCES = loadkeys.y analyze.l

YFLAGS = -d
LDADD = ../lib/libctlocal.a ../lib/libctutils.la ../lib/libconsole.la 

loadkeys_LDADD =  ../lib/libcfont.la $(LDADD) @LEXLIB@

bin_SCRIPTS = mk_modmap

noinst_HEADERS = loadkeys.h

EXTRA_DIST = $(bin_SCRIPTS)
