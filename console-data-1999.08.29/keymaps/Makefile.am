# -*- makefile -*-

AUTOMAKE_OPTIONS = foreign

include $(top_builddir)/Make.rules

CLEANFILES = $(KMAP_FILES:=.$(COMPRESSOR_EXT))

#pkgdata_DATA = $(Z_KMAP_FILES)
noinst_DATA = $(Z_KMAP_FILES)

#EXTRA_DIST = $(KMAP_FILES)

pkgdatadir = $(datadir)/$(KEYMAPDIR)

KMAP_DIRS = i386/qwertz i386/fgGIod i386/qwerty \
	i386/dvorak i386/azerty atari sun amiga mac

INC_DIRS = i386/include include

ALL_KDIRS = $(KMAP_DIRS) $(INC_DIRS)

KMAP_FILES = \
	$(patsubst $(srcdir)/%,%,$(wildcard $(patsubst %,$(srcdir)/%/*.kmap,$(KMAP_DIRS)))) \
	$(patsubst $(srcdir)/%,%,$(wildcard $(patsubst %,$(srcdir)/%/*.inc,$(INC_DIRS))))

Z_KMAP_FILES = $(KMAP_FILES:=.$(COMPRESSOR_EXT))

VPATH=$(srcdir):$(subst ,:, $(patsubst %,$(srcdir)/%,$(ALL_KDIRS)))

all: $(Z_KMAP_FILES)

install-data-local: all
	$(mkinstalldirs) $(patsubst %,$(DESTDIR)$(pkgdatadir)/%,$(ALL_KDIRS))
	for f in $(KMAP_FILES:=.$(COMPRESSOR_EXT)) ; do \
	  $(INSTALL_DATA) $$f $(DESTDIR)$(pkgdatadir)/$$(dirname $$f) ; \
	done

uninstall-local:
	for f in $(KMAP_FILES:=.$(COMPRESSOR_EXT)) ; do \
	  rm -f $(DESTDIR)$(pkgdatadir)/$$f ; \
	done
	-rmdir -p $(patsubst %,$(DESTDIR)$(pkgdatadir)/%,$(ALL_KDIRS)) 2>/dev/null

dist-hook:
	for f in $(KMAP_FILES) ; do \
	  $(mkinstalldirs) $(distdir)/$$(dirname $$f) ; \
	  $(INSTALL_DATA) $(srcdir)/$$f $(distdir)/$$(dirname $$f) ; \
	done
