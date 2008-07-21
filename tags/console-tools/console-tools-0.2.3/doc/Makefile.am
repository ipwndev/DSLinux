# -*- makefile -*-

AUTOMAKE_OPTIONS = foreign

SUBDIRS = man

EXTRA_DIST = lct.sgml lct.txt \
  kbd.FAQ.sgml kbd.FAQ.txt \
  README.* dvorak/* \
  file-formats/* contrib/*

DISTCLEANFILES = lct.dvi kbd.FAQ*.html kbd.FAQ.dvi

MAINTAINERCLEANFILES = lct*.html lct.txt kbd.FAQ.txt

all: lct.txt lct.html kbd.FAQ.txt kbd.FAQ.html

%.html: $(srcdir)/%.sgml
	sgml2html $<

%.txt: $(srcdir)/%.sgml
	sgml2txt $<

%.dvi: $(srcdir)/%.sgml
	sgml2latex $<
