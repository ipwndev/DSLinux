#
# build using configure as best we can, should work for most targets
# David McCullough <davidm@snapgear.com>
#

all: build
	$(MAKE) -C build

build: Makefile
	rm -rf build
	mkdir build
	chmod u+x configure
	( \
		cd build; \
		export CC="$(CC) $(CFLAGS)"; \
		export LDFLAGS="$(LDFLAGS)"; \
		export LIBS="$(LDLIBS)"; \
		../configure --host $(CROSS_COMPILE:-=) --prefix= ; \
	)

romfs:
	$(ROMFSINST) build/src/wget /bin/wget
	$(ROMFSINST) -m build/doc/wget.1 /usr/share/man/wget

clean:
	rm -rf build
	rm -f doc/texi2pod.pl
