# Makefile for console-tools

all: .compiled

CONSOLETOOLS_CONFIGURE_FLAGS=   --enable-static=yes --enable-shared=no \
				--disable-nls --libdir=$(ROOTDIR)/lib \
				--includedir=$(ROOTDIR)/include \
				--oldincludedir=$(ROOTDIR)/include \
				--disable-keycode-progs

.configured:
	cd src && env LIBS="-lintl -liconv"\
		./configure --host=arm-linux-elf \
		--prefix=/nonexistent $(CONSOLETOOLS_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:
	$(ROMFSINST) src/screenfonttools/consolechars /usr/bin
