## Makefile.am for libvideogfx/libvideogfx/graphics/basic

noinst_LTLIBRARIES = libvideogfx-graphics-basic.la

libvideogfx_graphics_basic_includedir = \
	$(includedir)/libvideogfx/graphics/basic

libvideogfx_graphics_basic_la_SOURCES = \
	bitmap.cc	\
	bitmap.hh	\
	image.cc	\
	image.hh	\
	image_tmpl.cc

libvideogfx_graphics_basic_include_HEADERS = \
	bitmap.hh	\
	image.hh

INCLUDES = \
	-I$(top_srcdir)

.PHONY: files

files:
	@files=`ls $(DISTFILES) 2> /dev/null`; for p in $$files; do \
	  echo $$p; \
	done
