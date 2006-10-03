## Makefile.am for libvideogfx/libvideogfx/graphics/draw

noinst_LTLIBRARIES = libvideogfx-graphics-draw.la

libvideogfx_graphics_draw_la_SOURCES = \
	bmformat.cc	\
	bmformat.hh	\
	draw.cc		\
	draw.hh

INCLUDES = \
	-I$(top_srcdir)

.PHONY: files

files:
	@files=`ls $(DISTFILES) 2> /dev/null`; for p in $$files; do \
	  echo $$p; \
	done
