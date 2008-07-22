# Makefile for tsmnc

all: .compiled

TMSNC_CONFIGURE_FLAGS=  --with-openssl=$(ROOTDIR) \
			--with-libiconv-prefix=$(ROOTDIR) \
			CC="ucfront-gcc arm-linux-elf-gcc"

.configured:
	cd src && env CFLAGS="$(CFLAGS)"\
		./configure --host=arm-linux-elf \
		--prefix=/nonexistent $(TMSNC_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:
	$(ROMFSINST) src/src/tmsnc /usr/bin
	$(ROMFSINST) -m src/doc/tmsnc.1 /usr/share/man/tmsnc
