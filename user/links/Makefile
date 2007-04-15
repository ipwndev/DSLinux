# Makefile for links

all: .compiled

LINKS_CONFIGURE_FLAGS=

ifeq ($(CONFIG_USER_LINKS_WITH_GRAPHICS),y)
LINKS_CONFIGURE_FLAGS+=--enable-graphics --with-fb \
		     --without-svgalib --without-x --without-directfb \
		     --without-sdl --without-pmshell --without-atheos \
		     --without-libtiff
endif

ifeq ($(CONFIG_USER_LINKS_WITH_GPM),y)
LINKS_CONFIGURE_FLAGS+=--with-gpm
endif

.configured:
	cd src && env "CFLAGS=$(CFLAGS) -Wno-pointer-sign" \
		./configure --target=$(CROSS) \
		--prefix=/nonexistent $(LINKS_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src clean
	rm -f src/config.cache
	rm -f .configured .compiled

# Empty romfs target since we only statically link with libraries
# and don't need to install them
romfs:
	$(ROMFSINST) src/links /bin/links
