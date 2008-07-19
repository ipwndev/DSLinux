## Makefile.am for libvideogfx/libvideogfx/graphics/fileio

libvideogfx_graphics_fileio_includedir = \
	$(includedir)/libvideogfx/graphics/fileio

if LINUX
noinst_LTLIBRARIES = \
	libvideogfx-graphics-fileio.la		\
	libvideogfx-graphics-fileio-linux.la
else
noinst_LTLIBRARIES = \
	libvideogfx-graphics-fileio.la
endif

libvideogfx_graphics_fileio_la_SOURCES = \
	read_yuv.cc	\
	read_yuv.hh	\
	rw_ppm.cc	\
	rw_ppm.hh	\
	rw_uyvy.cc	\
	rw_uyvy.hh	\
	write_yuv.cc	\
	write_yuv.hh

libvideogfx_graphics_fileio_include_HEADERS = \
	read_yuv.hh

if LINUX
libvideogfx_graphics_fileio_linux_la_SOURCES = \
	v4l_grab.cc	\
	v4l_grab.hh
endif

INCLUDES = \
	-I$(top_srcdir)

.PHONY: files

files:
	@files=`ls $(DISTFILES) 2> /dev/null`; for p in $$files; do \
	  echo $$p; \
	done
