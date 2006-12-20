DOOM_DIR=${shell pwd}/nxdoom

export CC=$(TARGET_CROSS)gcc
export CXX=$(TARGET_CROSS)g++
export AR=$(TARGET_CROSS)ar
export NM=$(TARGET_CROSS)nm
export RANLIB=$(TARGET_CROSS)ranlib
export LD=$(TARGET_CROSS)ld

$(DOOM_DIR)/.configured: 
	rm -f $(DOOM_DIR)/config.cache
	(cd $(DOOM_DIR); ./configure \
	--host=$(SYS) \
        --build=$(BUILD_SYS) \
	--with-microwin=$(strip $(subst ",, $(MW_PREFIX)))) 
	touch $(DOOM_DIR)/.configured

$(DOOM_DIR)/nxdoom: $(DOOM_DIR)/.configured
	make -C $(DOOM_DIR)

all: $(DOOM_DIR)/nxdoom

clean:
	if [ -f $(DOOM_DIR)/Makefile ]; then \
		make -C $(DOOM_DIR) distclean; \
	fi
	rm -f $(DOOM_DIR)/.configured

install: $(DOOM_DIR)/nxdoom
	mkdir -p $(INSTALL_DIR)/share/doom/
	cp doom1.wad $(INSTALL_DIR)/share/doom/
	cp $(DOOM_DIR)/doom $(INSTALL_DIR)/bin/nxdoom

.PHONY: all clean install
