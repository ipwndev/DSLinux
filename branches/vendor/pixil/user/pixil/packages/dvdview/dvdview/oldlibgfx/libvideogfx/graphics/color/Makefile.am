## Makefile.am for libvideogfx/libvideogfx/graphics/color

noinst_LTLIBRARIES = libvideogfx-graphics-color.la

libvideogfx_graphics_color_la_SOURCES = \
	colorspace.cc	\
	colorspace.hh

INCLUDES = \
	-I$(top_srcdir)

.PHONY: files

files:
	@files=`ls $(DISTFILES) 2> /dev/null`; for p in $$files; do \
	  echo $$p; \
	done
