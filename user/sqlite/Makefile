CFLAGS+=-DSQLITE_DISABLE_LFS=1
CFLAGS+=-Os

all: .compiled

.configured:
	cd src && ./configure --prefix=/usr --host=arm-linux-elf --disable-threadsafe --disable-dynamic-extensions
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-test -f src/Makefile && $(MAKE) -C src clean
	rm -f .configured .compiled src/config.cache

romfs: .compiled
	$(ROMFSINST) ./src/sqlite3 /usr/bin/sqlite3

.PHONY: all clean romfs

