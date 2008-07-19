if BUILD_ESDDSP
ESDDSP_MAN = esddsp.1
endif

man_MANS = \
	esd.1 esdcat.1 esd-config.1 esdctl.1 $(ESDDSP_MAN) \
	esdfilt.1 esdloop.1 esdmon.1 esdplay.1 esdrec.1 esdsample.1 

EXTRA_DIST = \
	esd.1.in esdcat.1.in esd-config.1.in esdctl.1.in \
	esddsp.1.in esdfilt.1.in esdloop.1.in esdmon.1.in \
	esdplay.1.in esdrec.1.in esdsample.1.in \
	esound.sgml

CLEANFILES = esound.dvi esound.ps esound.tex esound.aux esound.log

htmldir=$(pkgdocdir)/html

if HAVE_JW
noinst_DATA = html/index.html
else
noinst_DATA = 
endif

html/index.html: esound.sgml
	$(JW) -f docbook -b html -o html $(srcdir)/esound.sgml

clean-local:
	rm -rf html

if HAVE_JW
install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(htmldir)
	files=`cd html && echo *.html`; \
	test "$$files" && for f in $$files; do \
	  cp -p html/$$f $(DESTDIR)$(htmldir)/$$f; \
	done

uninstall-local:
	files=`cd html && echo *.html`; \
	test "$$files" && for f in $$files; do \
	  rm -rf $(DESTDIR)$(htmldir)/$$f; \
	done
	if test -d $(DESTDIR)$(htmldir); then rmdir $(DESTDIR)$(htmldir); fi
else
install-data-local:
uninstall-local:
endif


