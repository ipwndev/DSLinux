# Makefile for libtremor

all: .compiled

LIBTREMOR_CONFIGURE_FLAGS=	--disable-shared

.configured:
	cd src && env CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" \
		./configure --build=i386 --host=arm-linux-elf \
		--prefix=/usr $(LIBTREMOR_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src clean
	rm -f .configured .compiled

romfs:

