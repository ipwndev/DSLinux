all: .compiled
.configured:
	cd src && ./configure --prefix=/usr --host=${CROSS}
	touch $@
.compiled: .configured
	$(MAKE) -C src
	touch $@
clean:
	-test -f src/Makefile && $(MAKE) -C src clean
	rm -f .configured .compiled
romfs: .compiled
	$(ROMFSINST) ./src/bc/bc /usr/bin/bc
	$(ROMFSINST) ./src/dc/dc /usr/bin/dc
.PHONY: all clean romfs
