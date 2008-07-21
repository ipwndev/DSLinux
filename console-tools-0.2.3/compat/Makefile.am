# -*- makefile -*-

AUTOMAKE_OPTIONS = foreign

bin_SCRIPTS = setfont loadunimap saveunimap mapscrn

man_MANS = kbd-compat.8

EXTRA_DIST = $(MANS) $(SCRIPTS)

# install
install-data-local: do-install-man-@COMPAT@
do-install-man-nocompat:
	:
do-install-man-compat:
	for f in $(bin_SCRIPTS); do \
	  ln -sf kbd-compat.8 $(DESTDIR)$(mandir)/man8/$$f.8 ; \
	done

# uninstall
uninstall-local: do-uninstall-man-@COMPAT@
do-uninstall-man-nocompat:
	:
do-uninstall-man-compat:
	for f in $(bin_SCRIPTS); do \
	  rm -f $(DESTDIR)$(mandir)/man8/$$f.8 ; \
	done
