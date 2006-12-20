CONFOPTS=--with-linux=$(ROOTDIR)/$(LINUXDIR)

all: Makefile
	$(MAKE) -f Makefile

Makefile:
	./configure --host=$(CROSS_COMPILE:-=) $(CONFOPTS)

clean:
	$(MAKE) -f Makefile clean
	rm -f Makefile

romfs:
	$(ROMFSINST) brctl/brctl /bin/brctl
