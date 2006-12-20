## Makefile.am for libvideogfx/libvideogfx/graphics/filters

noinst_LTLIBRARIES = libvideogfx-graphics-filters.la

libvideogfx_graphics_filters_la_SOURCES = \
	linear.cc	\
	linear.hh

INCLUDES = \
	-I$(top_srcdir)

.PHONY: files

files:
	@files=`ls $(DISTFILES) 2> /dev/null`; for p in $$files; do \
	  echo $$p; \
	done
