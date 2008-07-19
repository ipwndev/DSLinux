# Makefile for gzip

all: .compiled

GZIP_CONFIGURE_FLAGS=--disable-largefile --enable-static=yes --enable-shared=no

.configured:
	cd src && env CFLAGS="$(CFLAGS)"\
		./configure --host=arm-linux-elf \
		--prefix=/nonexistent $(GZIP_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:
	$(ROMFSINST) src/gzip /bin/gzip
	$(ROMFSINST) -m src/gzip.1 /usr/share/man/gzip
