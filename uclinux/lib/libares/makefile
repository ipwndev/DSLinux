CONFOPTS := 

all: build/build
	$(MAKE) -C build
	$(RANLIB) build/libares.a

build/build: makefile
	@chmod u+x configure
	@rm -rf build
	@mkdir build
	(cd build; ../configure --host=$(CROSS_COMPILE:-=) $(CONFOPTS))
	@touch build/build

clean:
	rm -rf build

romfs:
