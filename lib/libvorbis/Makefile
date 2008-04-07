# Makefile for libvorbis

all: .compiled

LIBVORBIS_CONFIGURE_FLAGS=	--disable-shared --disable-oggtest \
				--with-ogg-libraries=$(ROOTDIR)/lib \
				--with-ogg-includes=$(ROOTDIR)/include \

.configured:
	cd src && env CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" \
		./configure --build=i386 --host=arm-linux-elf \
		--prefix=/usr $(LIBVORBIS_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src clean
	rm -f .configured .compiled

romfs:

