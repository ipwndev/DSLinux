# -*- makefile -*-

AUTOMAKE_OPTIONS = foreign

include $(top_builddir)/Make.rules

SUFFIXES = .fallback .nofallback

MK_FALLBACK = $(top_srcdir)/local-scripts/make-fallbacks.pl
MK_NOFALLBACK = $(top_srcdir)/local-scripts/make-nofallbacks.pl

EXTRA_DIST = $(SFM_FILES) $(ACM_FILES) $(FB_FILES) $(OLD_ACM_FILES) \
	UTF-tmpl.8bit UTF-tmpl.8bit.in

pkgdatadir = $(datadir)/$(TRANSDIR)

unidatadir = ${top_srcdir}/unidata

pkgdata_DATA = $(SFM_FILES:=.$(COMPRESSOR_EXT)) $(ACM_FILES:=.$(COMPRESSOR_EXT)) \
	 $(FB_FILES:=.$(COMPRESSOR_EXT)) $(OLD_ACM_FILES)

CLEANFILES= $(SFM_FILES:=.$(COMPRESSOR_EXT)) $(ACM_FILES:=.$(COMPRESSOR_EXT)) \
	 $(FB_FILES:=.$(COMPRESSOR_EXT))

AUTO_FALLBACKS = latin_diacr arabic_forms

AUTO_FALLBACKS_FB = $(AUTO_FALLBACKS:=.fallback)
AUTO_FALLBACKS_NFB = $(AUTO_FALLBACKS:=.nofallback)

# We don't put these ones in distclean because it's quite
# heavy to recompute.
MAINTAINERCLEANFILES = $(AUTO_FALLBACKS:=.fallback) $(AUTO_FALLBACKS:=.nofallback)

latin_diacr_PATTERN='^LATIN (SMALL|CAPITAL) (LETTER|LIGATURE) ((?:LONG|OPEN|DOTLESS) )?([^ ;]+)( DIGRAPH)?(?:( WITH ([^;]+?))( AND ([^;]+))?)?$$'
latin_diacr_TRANSLATIONS='LATIN $$1 $$2 $$3$$4$$5$$6' 'LATIN $$1 $$2 $$3$$4$$5' 'LATIN $$1 $$2 $$4$$5'
latin_diacr_HOPEFULL_PATTERN='^LATIN'

arabic_forms_PATTERN='^ARABIC LETTER ([^ ;]+)(?: (ISOLATED|INITIAL|MEDIAL|FINAL) FORM)?$$'
arabic_forms_TRANSLATIONS='ARABIC LETTER $$1'
arabic_forms_HOPEFULL_PATTERN='^ARABIC LETTER [^;]+ (ISOLATED|INITIAL|MEDIAL|FINAL) FORM$$'

fallbacks: $(AUTO_FALLBACKS:=.fallback) $(AUTO_FALLBACKS:=.nofallback)
force-fallbacks:
	rm -f $(AUTO_FALLBACKS:=.fallback) $(AUTO_FALLBACKS:=.nofallback)
	make fallbacks

#$(AUTO_FALLBACKS:=.fallback) : %.fallback : ../UnicodeData-2.txt $(MK_FALLBACK)
$(AUTO_FALLBACKS_FB) : %.fallback : ${unidatadir}/UnicodeData-2.txt $(MK_FALLBACK)
	@echo
	$(MK_FALLBACK) $($*_PATTERN) $($*_TRANSLATIONS) < $< > $@

#$(AUTO_FALLBACKS:=.nofallback) : %.nofallback : ../UnicodeData-2.txt $(MK_FALLBACK)
$(AUTO_FALLBACKS_NFB) : %.nofallback : ${unidatadir}/UnicodeData-2.txt $(MK_FALLBACK)
	@echo
	$(MK_NOFALLBACK) $($*_PATTERN) $($*_HOPEFULL_PATTERN) < $< > $@

SFM_FILES = \
iso01.sfm iso02.sfm iso03.sfm iso04.sfm iso05.sfm iso06.sfm \
iso07.sfm iso08.sfm iso09.sfm iso10.sfm iso14.sfm iso15.sfm \
ECMA144.sfm cp437.sfm def.sfm ethiopic.sfm koi8-r.sfm \
lat1.sfm lat1u.sfm lat2u.sfm lat4.sfm lat4u.sfm \
armscii8.sfm cp850.sfm cp850b.sfm tcvn.sfm viscii.sfm \
koi8u.sfm ruscii.sfm

OLD_ACM_FILES = null.trans space.trans trivial.trans zero.trans \
vga2iso.trans koi2alt.trans koi8u2ruscii.trans \
cp437_to_iso01.trans cp850_to_iso01.trans \
viscii1.0_to_tcvn.trans viscii1.0_to_viscii1.1.trans

ACM_FILES = \
cp437.acm iso01.acm iso02.acm iso03.acm iso04.acm iso05.acm iso06.acm \
iso07.acm iso08.acm iso09.acm iso10.acm straight-to-font.acm \
iso15.acm iso02+euro.acm koi8u.acm ruscii.acm


FB_FILES = \
$(AUTO_FALLBACKS_FB) $(AUTO_FALLBACKS_NFB) \
graph.fallback math.fallback misc.fallback \
typo.fallback qrczak.fallback
