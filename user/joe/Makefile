# Makefile for joe

all: .compiled

JOE_CONFIGURE_FLAGS=--disable-largefile

.configured:
	cd src && env CFLAGS="$(CFLAGS)"\
		./configure --host=arm-linux-elf \
		--prefix=/nonexistent $(JOE_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:
	$(ROMFSINST) src/joe /usr/bin/joe
	$(ROMFSINST) -m src/man/joe.1 /usr/share/man/joe
