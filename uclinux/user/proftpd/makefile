CONFOPTS=--enable-cap --prefix= --with-libraries=$(ROOTDIR)/user/proftpd/build/lib 

ifndef CONFIG_USER_TINYLOGIN_SHADOWPASSWDS
CONFOPTS += --disable-shadow
endif

all: build/build
	$(MAKE) -C build

build/build: makefile
	rm -rf build
	mkdir build
	(cd build; ../configure --host=$(CROSS_COMPILE:-=) $(CONFOPTS))
	mkdir -p build/lib/libcap
	cp lib/libcap/Makefile build/lib/libcap/
	touch build/build

clean:
	rm -rf build

romfs:
	$(ROMFSINST) build/proftpd /bin/profptd
