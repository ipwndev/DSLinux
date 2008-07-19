# Makefile for xrick

all: .compiled

.compiled: 
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src clean
	rm -f .compiled

romfs:
	$(ROMFSINST) src/xrick /usr/games/xrick
	$(ROMFSINST) -d data.zip /usr/share/games/xrick/data.zip

