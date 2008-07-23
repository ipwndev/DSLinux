# Makefile for diffutils

all: .compiled

DIFFUTILS_CONFIGURE_FLAGS=--disable-largefile --disable-nls

.configured:
	cd src && env CFLAGS="$(CFLAGS)" \
		./configure --host=arm-linux-elf \
		--prefix=/nonexistent $(DIFFUTILS_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:
	$(ROMFSINST) -e CONFIG_USER_DIFFUTILS_CMP src/src/cmp /usr/bin/
	$(ROMFSINST) -e CONFIG_USER_DIFFUTILS_DIFF src/src/diff /usr/bin/
