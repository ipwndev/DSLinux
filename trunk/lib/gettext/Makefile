# Makefile for gettext

all: .compiled

LIBGETTEXT_CONFIGURE_FLAGS=--enable-static --disable-shared --disable-java --disable-native-java --with-libiconv-prefix=$(ROOTDIR) --with-included-gettext --without-emacs 



.configured:
	cd src && env CFLAGS="$(CFLAGS)" \
		./configure --build=i386 --host=none-uclinux \
		--prefix=/usr $(LIBGETTEXT_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src clean
	rm -f .configured .compiled

romfs:

