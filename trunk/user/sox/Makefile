# Makefile for sox

all: .compiled

SOX_CONFIGURE_FLAGS=--disable-largefile --disable-nls \
		    --enable-static --disable-shared

.configured:
	cd src && env CFLAGS="$(CFLAGS)"\
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
