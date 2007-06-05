## Makefile.am for libvideogfx/libvideogfx/x11

noinst_LTLIBRARIES = libvideogfx-x11.la

libvideogfx_x11_la_SOURCES = \
	dispimg.cc	\
	dispimg.hh	\
	imgwin.cc	\
	imgwin.hh	\
	server.cc	\
	server.hh

INCLUDES = \
	-I$(top_srcdir)

.PHONY: files

files:
	@files=`ls $(DISTFILES) 2> /dev/null`; for p in $$files; do \
	  echo $$p; \
	done
