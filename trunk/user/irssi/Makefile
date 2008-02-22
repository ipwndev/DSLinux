# Makefile for irssi

all: .compiled

IRSSI_CONFIGURE_FLAGS=--enable-static --disable-shared --with-ncurses=$(ROOTDIR)/lib/libncurses --disable-glibtest --with-perl-staticlib --with-perl=yes --enable-ipv6=no LDFLAGS="$(LDFLAGS) -L$(ROOTDIR)/lib -lintl -liconv" CFLAGS="-I$(ROOTDIR)/include $(CFLAGS)"


.configured:
	cd src && env CFLAGS="$(CFLAGS)" \
		 PKG_CONFIG=no \
		./configure --build i386 --host none-uclinux-uclibc  \
		--prefix=/usr $(IRSSI_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:
	$(ROMFSINST) src/src/fe-text/irssi /bin/irssi
	$(ROMFSINST) -m docs/irssi.1 /usr/share/man/irssi
