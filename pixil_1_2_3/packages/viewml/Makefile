VIEWML_DIR=${shell pwd}/viewml

export CC=$(TARGET_CROSS)gcc
export CXX=$(TARGET_CROSS)g++
export AR=$(TARGET_CROSS)ar
export NM=$(TARGET_CROSS)nm
export RANLIB=$(TARGET_CROSS)ranlib
export LD=$(TARGET_CROSS)ld

$(VIEWML_DIR)/.configured: 
	(cd $(VIEWML_DIR); ./configure \
	--host=$(SYS) \
        --build=$(BUILD_SYS) \
	--with-libwwwconf=$(strip $(subst ",, $(LIBWWW_PREFIX)))/bin \
	--with-microwin=$(strip $(subst ",, $(MW_PREFIX))) \
	--with-fltk=$(BASE_DIR)/libs/flnx \
	--with-nxapp=$(BASE_DIR)/libs/pim)
	touch $(VIEWML_DIR)/.configured

$(VIEWML_DIR)/src/viewml: $(VIEWML_DIR)/.configured
	make -C $(VIEWML_DIR) -e

all: $(VIEWML_DIR)/src/viewml

clean: 
	if [ -f $(VIEWML_DIR)/Makefile ]; then \
		make -C $(VIEWML_DIR) distclean; \
	fi
	rm -rf $(VIEWML_DIR)/.configured

install: $(VIEWML_DIR)/src/viewml
	cp ./viewml/src/viewml $(INSTALL_DIR)/bin

.PHONY: all clean install
