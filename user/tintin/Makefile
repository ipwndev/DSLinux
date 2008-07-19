# Makefile for tintin++

.PHONY: all clean romfs

all: .compiled

.configured:
	cd src && ./configure --prefix=/usr --host=${CROSS}
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-test -f src/Makefile && $(MAKE) -C src clean distclean
	rm -f .configured .compiled

distclean: clean

romfs: .compiled
	$(ROMFSINST) ./src/tt++ /bin/tt++
