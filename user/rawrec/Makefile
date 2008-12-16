# Makefile for rawrec

all: .compiled

.compiled:
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .compiled

romfs:
	$(ROMFSINST) src/rawrec /usr/bin/rawplay
	$(ROMFSINST) src/rawrec /usr/bin/rawrec
	$(ROMFSINST) -m docs/user/rawrec.1 /usr/share/man/rawrec
