# Makefile for irssi

all: .compiled

IRSSI_CONFIGURE_FLAGS=--enable-static --disable-shared --with-ncurses=$(ROOTDIR)/lib/libncurses --disable-glibtest --with-perl-staticlib --with-perl=yes --enable-ipv6=no LDFLAGS="$(LDFLAGS) -L$(ROOTDIR)/lib -lintl -liconv" CFLAGS="-I$(ROOTDIR)/include $(CFLAGS)"


.configured:
	cd src && env CFLAGS="$(CFLAGS)" \
		./configure --build i386 --host none-uclinux-uclibc  \
		--prefix=/usr $(IRSSI_CONFIGURE_FLAGS)
	sed -i -r 's@\-ldl@-lintl -liconv@' src/src/fe-text/Makefile
	touch $@

.compiled: .configured
	sed -i -r 's@\-ldl@-lintl -liconv@' src/src/fe-text/Makefile
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:
	$(ROMFSINST) src/src/fe-text/irssi /bin/irssi
