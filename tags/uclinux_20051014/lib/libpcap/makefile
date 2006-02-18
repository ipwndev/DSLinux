CONFOPTS=--with-pcap=linux --with-headers=$(ROOTDIR)/include

all: build/build
	$(MAKE) -C build

build/build: makefile
	rm -rf build
	mkdir build
	(cd build; ../configure --host=$(CROSS_COMPILE:-=) $(CONFOPTS))
	touch build/build

clean:
	rm -rf build

romfs:
