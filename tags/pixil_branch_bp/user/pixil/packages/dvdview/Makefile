DVDVIEW_DIR=${shell pwd}/dvdview

export CC=$(TARGET_CROSS)gcc
export CXX=$(TARGET_CROSS)g++
export AR=$(TARGET_CROSS)ar
export NM=$(TARGET_CROSS)nm
export RANLIB=$(TARGET_CROSS)ranlib
export LD=$(TARGET_CROSS)ld

$(DVDVIEW_DIR)/.configured: 
	(cd $(DVDVIEW_DIR)/libvideogfx; ./configure \
	--host=$(SYS) \
        --build=$(BUILD_SYS) \
	--with-nanox=$(strip $(subst ",, $(MW_PREFIX))))
	(cd $(DVDVIEW_DIR); ./configure \
	--host=$(SYS) \
        --build=$(BUILD_SYS) \
	--with-nanox=$(strip $(subst ",, $(MW_PREFIX))))
	touch $(DVDVIEW_DIR)/.configured
	cp $(DVDVIEW_DIR)/libvideogfx/libtool.orig $(DVDVIEW_DIR)/libvideogfx/libtool
	chmod +x $(DVDVIEW_DIR)/libvideogfx/libtool

$(DVDVIEW_DIR)/libvideogfx/libvideogfx.a: $(DVDVIEW_DIR)/.configured
	make -C $(DVDVIEW_DIR)/libvideogfx

$(DVDVIEW_DIR)/src/dvdview: $(DVDVIEW_DIR)/libvideogfx/libvideogfx.a
	make -C $(DVDVIEW_DIR)/src

all: $(DVDVIEW_DIR)/src/dvdview

clean:
	if [ -f $(DVDVIEW_DIR)/libvideogfx/Makefile ]; then \
		make -C $(DVDVIEW_DIR)/libvideogfx distclean; \
	fi

	if [ -f $(DVDVIEW_DIR)/src/Makefile ]; then \
		make -C $(DVDVIEW_DIR)/src clean; \
	fi
	rm -rf $(DVDVIEW_DIR)/libvideogfx/config.cache
	rm -rf $(DVDVIEW_DIR)/config.cache
	rm -f $(DVDVIEW_DIR)/config.log
	rm -f $(DVDVIEW_DIR)/config.status
	rm -f $(DVDVIEW_DIR)/.configured
	rm -f $(DVDVIEW_DIR)/libvideogfx/libtool
	rm -f $(DVDVIEW_DIR)/libvideogfx/libvideogfx/qt/Makefile
	rm -f $(DVDVIEW_DIR)/libvideogfx/libvideogfx/x11/Makefile

install: $(DVDVIEW_DIR)/src/dvdview
	cp $(DVDVIEW_DIR)/src/dvdview $(INSTALL_DIR)/bin


