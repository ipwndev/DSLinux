FLASH_DIR=${shell pwd}/flash

export CC=$(TARGET_CROSS)gcc
export CXX=$(TARGET_CROSS)g++
export AR=$(TARGET_CROSS)ar
export NM=$(TARGET_CROSS)nm
export RANLIB=$(TARGET_CROSS)ranlib
export LD=$(TARGET_CROSS)ld


$(FLASH_DIR)/nxflashplay: 
	make -C $(FLASH_DIR) NANOXDIR=$(strip $(subst ",, $(MW_PREFIX))) \
	RGB555=$(CONFIG_NXFLASH_RGB555) -e

all: $(FLASH_DIR)/nxflashplay

clean:
	make -C $(FLASH_DIR) clean

install: $(FLASH_DIR)/nxflashplay
	cp $(FLASH_DIR)/nxflashplay $(INSTALL_DIR)/bin
	mkdir -p $(INSTALL_DIR)/share/flash
	cp $(FLASH_DIR)/Test/*.swf $(INSTALL_DIR)/share/flash


