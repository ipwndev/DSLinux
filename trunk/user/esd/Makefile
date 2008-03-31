# Makefile for esd

all: .compiled

ESD_CONFIGURE_FLAGS=    --disable-alsa --enable-static=yes\
			--enable-shared=no --disable-nls \
			--disable-ipv6

.configured:
	cd src && env CFLAGS="$(CFLAGS)" \
		PKG_CONFIG="no" \
		AUDIOFILE_LIBS="-L$(ROOTDIR)/lib -laudiofile -lm" \
		AUDIOFILE_CFLAGS="$(CFLAGS)" \
		./configure --host=arm-linux-elf \
		--prefix=/nonexistent $(ESD_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	$(CC) $(CFLAGS) $(LDFLAGS) getip.c -o getip
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled
	rm getip.gdb getip

romfs:
	$(ROMFSINST) getip /bin/getip
	$(ROMFSINST) src/esd /bin/esd
	$(ROMFSINST) src/esdcat /bin/esdcat
	$(ROMFSINST) src/esdctl /bin/esdctl
	$(ROMFSINST) src/esdfilt /bin/esdfilt
	$(ROMFSINST) src/esdloop /bin/esdloop
	$(ROMFSINST) src/esdmon /bin/esdmon
	$(ROMFSINST) src/esdplay /bin/esdplay
	$(ROMFSINST) src/esdrec /bin/esdrec
	$(ROMFSINST) src/esdsample /bin/esdsample

	$(ROMFSINST) -m src/docs/esd.1 /usr/share/man/esd
	$(ROMFSINST) -m src/docs/esdcat.1 /usr/share/man/esdcat
	$(ROMFSINST) -m src/docs/esdctl.1 /usr/share/man/esdctl
	$(ROMFSINST) -m src/docs/esdfilt.1 /usr/share/man/esdfilt
	$(ROMFSINST) -m src/docs/esdloop.1 /usr/share/man/esdloop
	$(ROMFSINST) -m src/docs/esdmon.1 /usr/share/man/esdmon
	$(ROMFSINST) -m src/docs/esdplay.1 /usr/share/man/esdplay
	$(ROMFSINST) -m src/docs/esdrec.1 /usr/share/man/esdrec
	$(ROMFSINST) -m src/docs/esdsample.1 /usr/share/man/esdsample


