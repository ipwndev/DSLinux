## Makefile.am for libvideogfx/libvideogfx/containers

noinst_LTLIBRARIES = libvideogfx-containers.la

libvideogfx_containers_includedir = $(includedir)/libvideogfx/containers

# Header files for public installation (non-generated)
libvideogfx_containers_public_h_sources = \
	heap.hh

# Header files that don't get installed
libvideogfx_containers_private_h_sources = \
	array.hh	\
	array.cc	\
	array2.cc	\
	array2.hh	\
	queue.cc	\
	queue.hh

# C sources to build the library from
libvideogfx_containers_c_sources = \
	heap.cc

#
# setup sources and their dependancies
#
libvideogfx_containers_include_HEADERS = $(libvideogfx_containers_public_h_sources)
libvideogfx_containers_la_SOURCES = $(libvideogfx_containers_c_sources)
EXTRA_HEADERS +=
EXTRA_DIST += $(libvideogfx_containers_private_h_sources)

INCLUDES = \
	-I$(top_srcdir)

.PHONY: files

files:
	@files=`ls $(DISTFILES) 2> /dev/null`; for p in $$files; do \
	  echo $$p; \
	done
