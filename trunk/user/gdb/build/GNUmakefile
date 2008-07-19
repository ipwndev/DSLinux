# Makefile for gdbserver in DSLINUX
# =================================

# Variables used as arguments to configure

# CFLAGS  - set by environment
# LDFLAGS - set by environment
LIBS = $(LDLIBS)

.PHONY: all
all: .compiled	


.compiled: .config
	$(MAKE) -f Makefile V=1
	touch $@

.config:
	../gdb/gdbserver/configure CC="$(CC)" --build=i686-pc-linux-gnu --host=arm-linux
	touch $@
	

.PHONY: clean
clean:
	-rm .config .compiled
	-$(MAKE) -f Makefile clean
	-rm config.h config.log config.status Makefile a.out.gdb gdbreplay.gdb gdbserver.gdb stamp-h

.PHONY: romfs
romfs:
	$(ROMFSINST) /bin/gdbserver
