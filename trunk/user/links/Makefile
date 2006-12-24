# Makefile for links

all: .compiled

.configured:
	cd src && ./configure --target=$(CROSS) \
		--prefix=/nonexistent
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src clean
	rm -f .configured .compiled

# Empty romfs target since we only statically link with libraries
# and don't need to install them
romfs:
	$(ROMFSINST) src/links /bin/links
