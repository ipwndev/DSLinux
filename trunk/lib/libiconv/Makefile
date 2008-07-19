# Makefile for libiconv

all: .compiled

LIBICONV_CONFIGURE_FLAGS=--enable-static --disable-shared --with-libiconv-prefix=$(ROOTDIR) --with-libintl-prefix=$(ROOTDIR)


.configured:
	cd src && env CFLAGS="$(CFLAGS)" \
		./configure --build=i386 --host=none-uclinux \
		--prefix=/usr $(LIBICONV_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	ln -f -s $(ROOTDIR)/lib/libiconv/src/include/iconv.h $(ROOTDIR)/include/iconv.h 
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src clean
	rm -f .configured .compiled

romfs:

