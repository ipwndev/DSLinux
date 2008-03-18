# Makefile for ncmpc

all: .compiled

NCMPC_CONFIGURE_FLAGS=--disable-nls

.configured:
	cd src && env CFLAGS="$(CFLAGS)"\
		./configure --host=arm-linux-elf \
		--prefix=/nonexistent $(NCMPC_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:
	$(ROMFSINST) src/src/ncmpc /bin/ncmpc
	$(ROMFSINST) -m src/doc/ncmpc.1 /usr/share/man/ncmpc
