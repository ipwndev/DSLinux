# Makefile for mc

all: .compiled

MC_CONFIGURE_FLAGS=--without-included-slang --without-vfs \
		   --with-ncurses=$(ROOTDIR)/lib/libncurses \
		   --disable-nls

ifeq ($(CONFIG_USER_GPM_GPM),y)
MC_CONFIGURE_FLAGS+=--with-gpm-mouse
endif

.configured:
	cd src && env CFLAGS="$(CFLAGS) -DRENAMED_NCURSES" \
		./configure --target=$(CROSS) \
		--prefix=/nonexistent $(MC_CONFIGURE_FLAGS)
	sed -i -e 's:\(#define HAVE_GRANTPT\)://\1:' src/config.h
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:
	$(ROMFSINST) src/src/mc /bin/mc
	$(ROMFSINST) -m src/doc/mc.1 /usr/share/man/mc
