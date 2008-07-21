# -*- makefile -*-

AUTOMAKE_OPTIONS = foreign

FindfileLctDefs = -DDATADIR=\"$(datadir)\" -DTRANSDIR=\"$(TRANSDIR)\" \
	  -DFONTDIR=\"$(FONTDIR)\" -DKEYMAPDIR=\"$(KEYMAPDIR)\" \
	  -DVIDEOMODEDIR=\"$(VIDEOMODEDIR)\" -D__USE_GNU

INCLUDES = -I$(top_srcdir)/include $(FindfileLctDefs)

lib_LTLIBRARIES = libcfont.la libconsole.la libctutils.la

noinst_LIBRARIES = libctlocal.a

libctlocal_a_SOURCES = saveunimap.c miscutils.c

libctutils_la_SOURCES = findfile.c xfseek.c xfopen.c wrappers.c \
	sfm-utils.c unicode.c findfile-lct.c sfm-fallback.c

libcfont_la_SOURCES = sfm-rd-ascii.c sfm-wr-ascii.c \
	sfm-wr-binary.c sfm-rd-binary.c \
	sfm-misc.c psf-header.c \
	fontdata.c fontstruct.c

libconsole_la_SOURCES = acm.c misc-console-utils.c ksyms.c \
	kernelfont.c testUTF8.c


# Maybe one day libtool will support this:
# Deps between uninstalled libs:
#libcfont_la_LIBADD = ./libctutils.la
#libconsole_la_LIBADD = ./libctutils.la
