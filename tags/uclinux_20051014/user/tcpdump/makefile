CONFOPTS=

all: build/build
	$(MAKE) -C build

build/build: makefile
	rm -rf build
	mkdir build
	(cd build; sh ../configure --host=$(CROSS_COMPILE:-=) $(CONFOPTS))
	touch build/build

clean:
	rm -rf build

romfs:
	$(ROMFSINST) build/tcpdump /bin/tcpdump
