#
# Build using configure.
#

all: build/Makefile
	$(MAKE) -C build

build/Makefile: makefile
	-chmod u+x configure
	-rm -rf build
	-mkdir build
	( \
		cd build; \
		CC="$(CC) $(CFLAGS)" \
		LDFLAGS="$(LDFLAGS)" \
		LIBS="$(LDLIBS)" \
			../configure --host=$(CROSS_COMPILE:-=) \
				--enable-dependency-tracking \
				--disable-shared \
			; \
	)

clean:
	rm -rf build

romfs:
