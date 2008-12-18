# Makefile for audiofile

all: .compiled

AUDIOFILE_CONFIGURE_FLAGS= 	--disable-largefile --enable-static=yes \
				--enable-shared=no --prefix=$(ROOTDIR) \
				--enable-fast-install=no 

.configured:
	cd src && env CFLAGS="$(CFLAGS)"\
		./configure --host=arm-linux-elf \
		--prefix=/nonexistent $(AUDIOFILE_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled
.PHONY: romfs
romfs:
