## Makefile.am for libvideogfx

SUBDIRS = libvideogfx examples

bin_SCRIPTS = libvideogfx-config

EXTRA_DIST = \
	BUGS		\
	HACKING		\
	TODO		\
	autogen.sh	\
	libvideogfx.m4

m4datadir = $(datadir)/aclocal
m4data_DATA = libvideogfx.m4

DOCXXINPUT = \
	libvideogfx/graphics/basic/bitmap.hh	\
	libvideogfx/graphics/basic/image.hh

HTMLDOCDIR = htmldoc

.PHONY: files html dvi

html:
	doc++ -d $(HTMLDOCDIR) $(DOCXXINPUT)

dvi:
	doc++ -t -o doc.tex $(DOCXXINPUT)
	latex doc.tex
	dvips doc.dvi -o doc.ps

files:
	@files=`ls $(DISTFILES) 2> /dev/null `; for p in $$files; do \
	  echo $$p; \
	done
