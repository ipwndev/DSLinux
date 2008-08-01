# Makefile for rawrec

all: .compiled

.compiled:
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .compiled

romfs:
	#Installing as rawplay as we don't have a mic at the moment
	$(ROMFSINST) src/rawrec /usr/bin/rawplay
	$(ROMFSINST) -m docs/user/rawrec.1 /usr/share/man/rawplay
