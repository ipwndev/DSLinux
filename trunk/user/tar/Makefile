# Makefile for tar

all: .compiled

TAR_CONFIGURE_FLAGS=--disable-largefile --disable-nls --without-included-regex

.configured:
	cd src && env CFLAGS="$(CFLAGS)"\
		./configure --host=arm-linux-elf \
		--prefix=/nonexistent $(TAR_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:
	$(ROMFSINST) src/src/tar /bin/tar
