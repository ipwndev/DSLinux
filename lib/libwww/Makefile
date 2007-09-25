# Makefile for libwww

all: .compiled

LIBWWW_CONFIGURE_FLAGS=--without-expat --disable-shared


.configured:
	cd src && env CFLAGS="$(CFLAGS)" \
		./configure --target=$(CROSS) \
		--prefix=$(ROOTDIR) $(LIBWWW_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	chmod 755 src/libwww-config
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:

