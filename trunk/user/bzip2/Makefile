# Makefile for bzip2

all: .compiled

.configured:
	touch $@

.compiled: .configured
	$(MAKE) -C src bzip2 bzip2recover
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:
	$(ROMFSINST) src/bzip2 /bin/bzip2
	$(ROMFSINST) src/bzip2recover /bin/bzip2recover
	$(ROMFSINST) -m src/bzip2.1 /usr/share/man/bzip2
