## Makefile.am for libvideogfx/libvideogfx/graphics/lowlevel

if ENABLE_MMX
noinst_LTLIBRARIES = \
	libvideogfx-graphics-lowlevel.la	\
	libvideogfx-graphics-lowlevel-mmx.la
else
noinst_LTLIBRARIES = \
	libvideogfx-graphics-lowlevel.la
endif

libvideogfx_graphics_lowlevel_la_SOURCES = \
	img2raw.cc		\
	img2raw.hh		\
	yuv2rgb16.cc		\
	yuv2rgb16.hh

if ENABLE_MMX
libvideogfx_graphics_lowlevel_mmx_la_SOURCES = \
	grey2rgb16mmx.cc	\
	grey2rgb16mmx.hh	\
	grey2rgb32mmx.cc	\
	grey2rgb32mmx.hh	\
	yuv2rgb16mmx.cc		\
	yuv2rgb16mmx.hh		\
	yuv2rgb32mmx.cc		\
	yuv2rgb32mmx.hh
endif

INCLUDES = \
	-I$(top_srcdir)

.PHONY: files

files:
	@files=`ls $(DISTFILES) 2> /dev/null`; for p in $$files; do \
	  echo $$p; \
	done
