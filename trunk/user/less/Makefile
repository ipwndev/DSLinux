# Makefile for less

all: .compiled

LESS_CONFIGURE_FLAGS=--disable-largefile\


.configured:
	cd src &&\
		./configure --host=$(CROSS) \
		--prefix=/nonexistent $(LESS_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:
	$(ROMFSINST) src/less /bin/less
