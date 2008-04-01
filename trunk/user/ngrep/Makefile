# Makefile for ngrep

all: .compiled

NGREP_CONFIGURE_FLAGS=  --libdir=$(ROOTDIR)/lib --includedir=$(ROOTDIR)/include \
			--oldincludedir=$(ROOTDIR)/include \
			--disable-ipv6 --with-pcap-includes=$(ROOTDIR)/include \
			--with-dropprivs-user=root --enable-pcre

.configured:
	cd src && env CFLAGS="$(CFLAGS)"\
		./configure --host=arm-linux-elf \
		--prefix=/nonexistent $(NGREP_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:
	$(ROMFSINST) src/ngrep /usr/bin
	$(ROMFSINST) -m src/ngrep.8 /usr/share/man/ngrep
