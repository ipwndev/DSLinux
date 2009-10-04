all: .compiled

.configured:
	cd src && ./configure --prefix=/usr --host=${CROSS}
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-test -f src/Makefile && $(MAKE) -C src clean
	rm -f .configured .compiled src/config.cache

romfs: .compiled
	$(ROMFSINST) ./src/cmatrix /usr/bin/cmatrix
	$(ROMFSINST) ./src/matrix.fnt /usr/share/consolefonts/matrix.fnt

.PHONY: all clean romfs

