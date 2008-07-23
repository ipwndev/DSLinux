# Makefile for sed

all: .compiled

SED_CONFIGURE_FLAGS=--disable-largefile --disable-nls \
			--without-included-regex

.configured:
	cd src && env CFLAGS="$(CFLAGS)"\
		./configure --host=arm-linux-elf \
		--prefix=/nonexistent $(SED_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:
	arm-linux-elf-flthdr -s 8192 src/sed/sed
	$(ROMFSINST) src/sed/sed /bin/sed
