## Makefile.am for libvideogfx/libvideogfx/graphics/visualize

noinst_LTLIBRARIES = libvideogfx-graphics-visualize.la

libvideogfx_graphics_visualize_la_SOURCES = \
	markblks.cc	\
	markblks.hh	\
	motionfield.cc	\
	motionfield.hh	\
	regions.cc	\
	regions.hh

INCLUDES = \
	-I$(top_srcdir)

.PHONY: files

files:
	@files=`ls $(DISTFILES) 2> /dev/null`; for p in $$files; do \
	  echo $$p; \
	done
