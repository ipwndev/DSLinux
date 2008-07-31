# Makefile for Pipe Viewer

all: .compiled

PV_CONFIGURE_FLAGS=--disable-nls

.configured:
	cd src && env CFLAGS="$(CFLAGS)"\
		./configure --host=arm-linux-elf \
		--prefix=/nonexistent $(PV_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:
	$(ROMFSINST) src/pv /usr/bin/pv
	$(ROMFSINST) -m src/doc/quickref.1 /usr/share/man/pv
