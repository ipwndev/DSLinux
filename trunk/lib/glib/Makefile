# Makefile for libglib

all: .compiled

LIBGLIB_CONFIGURE_FLAGS=--enable-static --disable-shared --with-libiconv=gnu



.configured:
	cp -f src/config.cache.tmpl src/config.cache
	cd src && env CFLAGS="$(CFLAGS)" \
	./configure -C --cache-file=config.cache --build=i386 \
	--host=none-uclinux-uclibc \
	--prefix=/usr $(LIBGLIB_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src clean
	rm -f .configured .compiled

romfs:

