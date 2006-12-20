## Makefile.am for libvideogfx/libvideogfx

SUBDIRS = arch containers graphics utility nanox

INCLUDES = \
	-DDATADIR=\"$(datadir)\"	\
	-DLIBDIR=\"$(libdir)\"		\
	-DSYSCONFDIR=\"$(sysconfdir)\"	\
	-I$(top_srcdir)

#
# libraries to compile and install
#
lib_LTLIBRARIES = libvideogfx.la

# libtool stuff: set version and export symbols for resolving
libvideogfx_includedir = $(includedir)/libvideogfx
libvideogfx_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)	\
	-release $(LT_RELEASE)					\
	-export-dynamic

#
# setup sub-libraries variables
#
libvideogfx_common_ldadd = \
	containers/libvideogfx-containers.la			\
	graphics/basic/libvideogfx-graphics-basic.la		\
	graphics/color/libvideogfx-graphics-color.la		\
	graphics/datatypes/libvideogfx-graphics-datatypes.la	\
	graphics/draw/libvideogfx-graphics-draw.la		\
	graphics/fileio/libvideogfx-graphics-fileio.la		\
	graphics/filters/libvideogfx-graphics-filters.la	\
	graphics/lowlevel/libvideogfx-graphics-lowlevel.la	\
	graphics/motvec/libvideogfx-graphics-motvec.la		\
	graphics/visualize/libvideogfx-graphics-visualize.la	\
	utility/bitstream/libvideogfx-utility-bitstream.la

if LINUX
libvideogfx_linux_ldadd = \
	graphics/fileio/libvideogfx-graphics-fileio-linux.la
else
libvideogfx_linux_ldadd =
endif

if ENABLE_MMX
libvideogfx_mmx_ldadd = \
	arch/linux/x86/libvideogfx-arch-linux-x86.la		\
	graphics/lowlevel/libvideogfx-graphics-lowlevel-mmx.la
else
libvideogfx_mmx_ldadd =
endif

libvideogfx_qt_ldadd = \
	qt/libvideogfx-qt.la

libvideogfx_x11_ldadd = \
	nanox/libvideogfx-x11.la

libvideogfx_la_LIBADD = \
	$(libvideogfx_common_ldadd)	\
	$(libvideogfx_linux_ldadd)	\
	$(libvideogfx_mmx_ldadd)	\
	$(libvideogfx_x11_ldadd)

#
# setup source file variables
#
# Header files for public installation (non-generated)
libvideogfx_public_h_sources = \
	init.hh		\
	types.hh

# Header files that don't get installed
libvideogfx_private_h_sources =

# C sources to build the library from
libvideogfx_c_sources = \
	init.cc

#
# setup sources and their dependancies
#
libvideogfx_include_HEADERS = $(libvideogfx_public_h_sources)
libvideogfx_la_SOURCES = $(libvideogfx_c_sources)
EXTRA_HEADERS +=
EXTRA_DIST += $(libvideogfx_private_h_sources)

.PHONY: files

files:
	@files=`ls $(DISTFILES) 2> /dev/null `; for p in $$files; do \
	  echo $$p; \
	done
