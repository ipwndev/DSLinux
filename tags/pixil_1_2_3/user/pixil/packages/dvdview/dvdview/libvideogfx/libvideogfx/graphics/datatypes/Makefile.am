## Makefile.am for libvideogfx/libvideogfx/graphics/datatypes

noinst_LTLIBRARIES = libvideogfx-graphics-datatypes.la

libvideogfx_graphics_datatypes_la_SOURCES = \
	motionfield.cc	\
	motionfield.hh	\
	primitives.hh	\
	region.cc	\
	region.hh

INCLUDES = \
	-I$(top_srcdir)

.PHONY: files

files:
	@files=`ls $(DISTFILES) 2> /dev/null`; for p in $$files; do \
	  echo $$p; \
	done
