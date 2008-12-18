# Makefile for sox

all: .compiled

SOX_CONFIGURE_FLAGS=--disable-largefile --disable-nls \
		    --enable-static --disable-shared \
		    --with-samplerate --libdir=$(ROOTDIR)/lib \
		    --includedir=$(ROOTDIR)/include
.configured:
	cd src && env CFLAGS="$(CFLAGS)" PKG_CONFIG=no \
		./configure --host=arm-linux-elf \
		--prefix=/nonexistent $(SOX_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:
	$(ROMFSINST) src/src/sox /bin/sox
	$(ROMFSINST) -m src/sox.1 /usr/share/man/sox
