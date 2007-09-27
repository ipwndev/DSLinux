# Makefile for libwww

all: .compiled

LIBWWW_CONFIGURE_FLAGS=--without-expat --disable-shared


.configured:
	cd src && env CFLAGS="$(CFLAGS)" \
		./configure --target=$(CROSS) \
		--prefix=$(ROOTDIR) $(LIBWWW_CONFIGURE_FLAGS)
	touch $@

.compiled: #.configured
	$(MAKE) -C src
	sed -i 's@THEROOTDIR@$(ROOTDIR)@' src/libwww-config
	chmod 755 src/libwww-config
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src clean
	sed -i 's@$(ROOTDIR)@THEROOTDIR@' src/libwww-config
	rm -f .configured .compiled

romfs:

