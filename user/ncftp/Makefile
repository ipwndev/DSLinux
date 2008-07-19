# Makefile for ncftp

all: .compiled

NCFTP_CONFIGURE_FLAGS=	--host=arm-linux-elf --libdir=$(ROOTDIR)/lib \
			--includedir=$(ROOTDIR)/include --disable-largefile \
			--enable-static=yes --enable-shared=no --prefix=/usr/bin


.configured:
	cd src && ./configure $(NCFTP_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:
	arm-linux-elf-flthdr -s 512000 src/bin/ncftp.gdb
	$(ROMFSINST) src/bin/ncftp.gdb /bin/ncftp
	$(ROMFSINST) -m src/doc/man/ncftp.1 /usr/share/man/ncftp
