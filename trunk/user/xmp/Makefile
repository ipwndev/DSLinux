# Makefile for xmp

all: .compiled

XMP_CONFIGURE_FLAGS=--enable-oss --disable-alsa

.configured:
	cd src && \
		./configure --host=arm-linux-elf $(XMP_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:
	$(ROMFSINST) src/src/main/xmp /bin/xmp
	$(ROMFSINST) -m src/docs/xmp.1 /usr/share/man/xmp
