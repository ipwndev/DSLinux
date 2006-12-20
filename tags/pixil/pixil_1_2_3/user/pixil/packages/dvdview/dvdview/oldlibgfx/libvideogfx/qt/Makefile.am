## Makefile.am for libvideogfx/libvideogfx/qt

noinst_LTLIBRARIES = libvideogfx-qt.la

libvideogfx_qt_la_SOURCES = \
	imgwidget.cc	\
	imgwidget.hh

INCLUDES = \
	-I$(top_srcdir)

moc_%.cc: %.hh
	moc -f $< -o $@

.PHONY: files

files:
	@files=`ls $(DISTFILES) 2> /dev/null`; for p in $$files; do \
	  echo $$p; \
	done
