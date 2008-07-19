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
	$(ROMFSINST) -e CONFIG_USER_MANPAGES man /usr/bin/man
	$(ROMFSINST) -m src/less.nro /usr/share/man/less
