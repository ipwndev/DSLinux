# Makefile for libsndfile

all: .compiled

LIBSNDFILE_CONFIGURE_FLAGS= 	--enable-shared=no --enable-static=yes \
				--prefix=$(ROOTDIR) --disable-largefile \
				--disable-alsa --disable-sqlite

.configured:
	cd src && env CFLAGS="$(CFLAGS)"\
		./configure --host=arm-linux-elf \
		--prefix=/nonexistent $(LIBSNDFILE_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled
.PHONY: romfs
romfs:
