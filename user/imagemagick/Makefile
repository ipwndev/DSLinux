# Makefile for ImageMagick

all: .compiled

BIN=animate compare composite conjure convert display \
	identify import mogrify montage stream

IMAGEMAGICK_CONFIGURE_FLAGS=  --disable-largefile \
			--enable-static --disable-shared \
			--without-bzlib --without-dps \
			--without-fpx --without-freetype \
			--without-gslib --without-gvc \
			--without-jbig --without-jp2 \
			--without-lcms --without-openexr \
			--without-tiff --without-perl \
			--with-included-ltdl \
			--without-x --with-jpeg --enable-embeddable
.configured:
	cd src && env CFLAGS="$(CFLAGS)"\
		./configure --host=arm-linux-elf \
		--prefix=/nonexistent $(IMAGEMAGICK_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:

	@for i in $(BIN);\
	do \
		echo "installing $$i";\
		arm-linux-elf-flthdr -s 8192 src/utilities/$$i;\
		$(ROMFSINST) src/utilities/$$i /usr/bin/;\
	done
