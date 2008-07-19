# Makefile for ncmpc

all: .compiled

NCMPC_CONFIGURE_FLAGS=--disable-nls
.configured:
	cd src && env GLIB_CFLAGS="-I$(ROOTDIR)/include" \
		GLIB_LIBS="-L$(ROOTDIR)/lib -lglib-2.0 -liconv -lintl" \
		PKG_CONFIG="no" \
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
