# Makefile for e2fsprogs

all: .compiled

E2FSPROGS_CONFIGURE_FLAGS=--disable-largefile --disable-nls \
			  --libdir=$(ROOTDIR)/lib --includedir=$(ROOTDIR)/include \
			  --with-cc="$(CC)" --with-linker="$(LD)" \
			  --with-ccopts="$(CFLAGS)" --with-ldopts="$(LDFLAGS)"

.configured:
	cd src && env CFLAGS="$(CFLAGS)"\
		./configure --host=arm-linux-elf \
		--prefix=/nonexistent $(E2FSPROGS_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_E2FSCK_E2FSCK src/e2fsck/e2fsck /bin/e2fsck
	$(ROMFSINST) -e CONFIG_USER_E2FSPROGS_E2FSCK_E2FSCK -m src/e2fsck/e2fsck.8 /usr/share/man/e2fsck
