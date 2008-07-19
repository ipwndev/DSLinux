# Makefile for noiz2sa

all: .compiled

.compiled: 
	$(MAKE) -C src/bulletml/tinyxml
	$(MAKE)  -C src/bulletml
	$(MAKE) -C src
	arm-linux-elf-flthdr -s 16384 src/noiz2sa
	touch $@

clean:
	-[ -f src/bulletml/tinyxml/Makefile ] && $(MAKE) -C src/bulletml/tinyxml clean
	-[ -f src/bulletml/Makefile ] && $(MAKE) -C src/bulletml clean
	-[ -f src/Makefile ] && $(MAKE) -C src clean
	rm -f .compiled

romfs:
	$(ROMFSINST) src/noiz2sa /usr/games/noiz2sa
	$(ROMFSINST) -d noiz2sa_share /usr/share/games/noiz2sa/

