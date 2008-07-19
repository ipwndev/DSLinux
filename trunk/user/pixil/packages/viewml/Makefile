VIEWML_DIR=${shell pwd}/viewml

export BASE_DIR=$(ROOTDIR)/user/pixil
#export CC=$(CROSS)gcc
#export CXX=$(CROSS)g++
export AR=$(CROSS)ar
export NM=$(CROSS)nm
export RANLIB=$(CROSS)ranlib
export LD=$(CROSS)ld
export CFLAGS+=-Wno-deprecated

.configured: 
	(cd $(VIEWML_DIR); ./configure \
	--host=$(CROSS) \
	--with-libwwwconf=$(strip $(subst ",, $(ROOTDIR)))/lib/libwww/src \
	--with-microwin=$(strip $(subst ",, $(ROOTDIR)))/user/microwin/src \
	--with-fltk=$(BASE_DIR)/libs/flnx \
	--with-nxapp=$(BASE_DIR)/libs/pim)
	touch $@

.compiled: .configured
	make -C $(VIEWML_DIR) -e
	touch $@

all: .compiled

clean: 
	if [ -f $(VIEWML_DIR)/Makefile ]; then \
		make -C $(VIEWML_DIR) distclean; \
	fi
	rm -rf .configured
	rm -rf .compiled

install: $(VIEWML_DIR)/src/viewml
	$(CROSS)flthdr -z viewml/src/viewml
	cp ./viewml/src/viewml $(INSTALL_DIR)/bin

.PHONY: all clean install
