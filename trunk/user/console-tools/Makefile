# Makefile for console-tools

all: .compiled

CONSOLETOOLS_CONFIGURE_FLAGS=   --enable-static=yes --enable-shared=no \
				--disable-nls --libdir=$(ROOTDIR)/lib \
				--includedir=$(ROOTDIR)/include \
				--oldincludedir=$(ROOTDIR)/include \
				--disable-keycode-progs

.configured:
	cd src &&env LIBS="-lintl -liconv" CFLAGS="$(CFLAGS)"\
		./configure --host=arm-linux-elf \
		--prefix=/usr $(CONSOLETOOLS_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:
	
	$(ROMFSINST) -e CONFIG_USER_CONSOLETOOLS_FONTFILETOOLS \
			src/fontfiletools/psfaddtable /usr/bin
	$(ROMFSINST) -e CONFIG_USER_CONSOLETOOLS_FONTFILETOOLS \
			src/fontfiletools/psfgettable /usr/bin
	$(ROMFSINST) -e CONFIG_USER_CONSOLETOOLS_FONTFILETOOLS \
			src/fontfiletools/psfstriptable /usr/bin
	#Install .psf fonts if consolechars is enabled
	$(ROMFSINST) -e CONFIG_USER_CONSOLETOOLS_SCREENFONTTOOLS \
			consolefonts /usr/share/consolefonts
	$(ROMFSINST) -e CONFIG_USER_CONSOLETOOLS_SCREENFONTTOOLS \
			src/screenfonttools/consolechars /usr/bin
	$(ROMFSINST) -e CONFIG_USER_CONSOLETOOLS_SCREENFONTTOOLS \
			src/screenfonttools/showcfont /usr/bin
	$(ROMFSINST) -e CONFIG_USER_CONSOLETOOLS_VTTOOLS \
			src/vttools/chvt /usr/bin
	$(ROMFSINST) -e CONFIG_USER_CONSOLETOOLS_VTTOOLS \
			src/vttools/deallocvt /usr/bin
	$(ROMFSINST) -e CONFIG_USER_CONSOLETOOLS_VTTOOLS \
			src/vttools/fgconsole /usr/bin
	$(ROMFSINST) -e CONFIG_USER_CONSOLETOOLS_VTTOOLS \
			src/vttools/openvt /usr/bin
	$(ROMFSINST) -e CONFIG_USER_CONSOLETOOLS_VTTOOLS \
			src/vttools/screendump /usr/bin
	$(ROMFSINST) -e CONFIG_USER_CONSOLETOOLS_VTTOOLS \
			src/vttools/setvesablank /usr/bin
	$(ROMFSINST) -e CONFIG_USER_CONSOLETOOLS_VTTOOLS \
			src/vttools/vcstime /usr/bin
	$(ROMFSINST) -e CONFIG_USER_CONSOLETOOLS_VTTOOLS \
			src/vttools/vt-is-UTF8 /usr/bin
	$(ROMFSINST) -e CONFIG_USER_CONSOLETOOLS_VTTOOLS \
			src/vttools/writevt /usr/bin
