# Makefile for warp

all: .compiled

WARP_CONFIGURE_FLAGS=--with-sdl-prefix=$(ROOTDIR)

.configured:
	cd src && LIBS="-lm -lSDL -lpthread"\
		./configure --host=arm-linux-elf \
		--prefix=/nonexistent $(WARP_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:
	$(ROMFSINST) src/warp /usr/bin/
