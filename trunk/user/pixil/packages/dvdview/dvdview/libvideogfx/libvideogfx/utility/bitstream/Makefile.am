## Makefile.am for libvideogfx/utility/bitstream

libvideogfx_utility_bitstream_includedir = \
	$(includedir)/libvideogfx/utility/bitstream

noinst_LTLIBRARIES = libvideogfx-utility-bitstream.la

libvideogfx_utility_bitstream_la_SOURCES = \
	fastbitbuf.hh	\
	fastbitbuf.icc	\
	bitbuffer.hh \
	bitbuffer.cc \
	membitsread.cc	\
	membitsread.hh	\
	membitsread.icc

libvideogfx_utility_bitstream_include_HEADERS = \
	bitbuffer.hh

INCLUDES = \
	-I$(top_srcdir)

.PHONY: files

files:
	@files=`ls $(DISTFILES) 2> /dev/null`; for p in $$files; do \
	  echo $$p; \
	done
