## Makefile.am for libvideogfx/libvideogfx/graphics/motvec

noinst_LTLIBRARIES = libvideogfx-graphics-motvec.la

libvideogfx_graphics_motvec_la_SOURCES = \
	blkcmp.hh	\
	full.cc		\
	full.hh		\
	mv.cc		\
	mv.hh

INCLUDES = \
	-I$(top_srcdir)

.PHONY: files

files:
	@files=`ls $(DISTFILES) 2> /dev/null`; for p in $$files; do \
	  echo $$p; \
	done
