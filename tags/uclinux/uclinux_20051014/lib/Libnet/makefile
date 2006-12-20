CONFOPTS=--with-headers=$(ROOTDIR)/include --with-pf_packet=yes --with-fast_x86_check=no

all: Makefile
	[ -d lib ] || mkdir lib
	$(MAKE) -f Makefile

Makefile: makefile
	sh configure --host=$(CROSS_COMPILE:-=) $(CONFOPTS)

clean:
	if [ -f Makefile ]; then $(MAKE) -f Makefile distclean; fi

romfs:
