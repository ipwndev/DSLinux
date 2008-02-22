# Makefile for mktemp

all: .compiled

MKTEMP_CONFIGURE_FLAGS=--with-libc

.configured:
	cd src && env CFLAGS="$(CFLAGS)"\
		./configure --host=arm-linux-elf \
		--prefix=/nonexistent $(MKTEMP_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:
	$(ROMFSINST) src/mktemp /bin/mktemp
	$(ROMFSINST) -m src/mktemp.man /usr/share/man/mktemp
