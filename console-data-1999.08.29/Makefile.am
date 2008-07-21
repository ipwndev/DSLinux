# -*- makefile -*-

AUTOMAKE_OPTIONS = foreign

# turn around automake 1.2
COMPRESSOR = @COMPRESSOR@

SUBDIRS = . consolefonts consoletrans keymaps doc

EXTRA_DIST = unidata/* local-scripts/*

install-data-local:
	$(mkinstalldirs) \
	  $(DESTDIR)$(datadir)/$(VIDEOMODEDIR) \
	  $(DESTDIR)$(datadir)/unidata
	${INSTALL_DATA} ${top_srcdir}/unidata/* $(DESTDIR)$(datadir)/unidata/

uninstall-local:
	rm -rf $(DESTDIR)$(datadir)/unidata/

# try to have Make.rules rebuilt when needed
Makefile: Make.rules


fallbacks:
	make -C data/consoletrans fallbacks
force-fallbacks:
	make -C data/consoletrans force-fallbacks
