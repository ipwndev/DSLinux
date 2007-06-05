## Makefile.am for libvideogfx/examples

#
# test programs, not to be installed
#
noinst_PROGRAMS = test-libvideogfx bitstream_output

test_libvideogfx_DEPENDENCIES = ../libvideogfx/libvideogfx.la
test_libvideogfx_CFLAGS = $(X_CFLAGS)
test_libvideogfx_LDFLAGS = -g $(X_LDFLAGS)
test_libvideogfx_LDADD = $(X_LIBS) ../libvideogfx/libvideogfx.la
test_libvideogfx_SOURCES = test-libvideogfx.cc

bitstream_output_DEPENDENCIES = ../libvideogfx/libvideogfx.la
bitstream_output_CFLAGS = $(X_CFLAGS)
bitstream_output_LDFLAGS = -g $(X_LDFLAGS)
bitstream_output_LDADD = $(X_LIBS) ../libvideogfx/libvideogfx.la
bitstream_output_SOURCES = bitstream_output.cc

.PHONY: files

files:
	@files=`ls $(DISTFILES) 2> /dev/null `; for p in $$files; do \
	  echo $$p; \
	done
