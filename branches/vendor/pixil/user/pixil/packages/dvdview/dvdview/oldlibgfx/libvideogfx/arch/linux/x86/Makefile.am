## Makefile.am for libvideogfx/libvideogfx/arch/linux/x86

if ENABLE_MMX
noinst_LTLIBRARIES = libvideogfx-arch-linux-x86.la

libvideogfx_arch_linux_x86_la_SOURCES = \
	CPUcapabilities.cc
else
noinst_LTLIBRARIES =
endif

INCLUDES = \
	-I$(top_srcdir)

.PHONY: files

files:
	@files=`ls $(DISTFILES) 2> /dev/null`; for p in $$files; do \
	  echo $$p; \
	done
