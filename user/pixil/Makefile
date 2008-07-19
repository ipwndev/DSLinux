####
# Pixil Makefile
# Copyright 2002, Century Software
#
# Written by:  Jordan Crouse
# Released under the GPL (see LICENSE.GPL) for details
####

-include config

# Bring in the configuration file

BASE_DIR    = ${shell pwd}
STAGE_DIR   = $(BASE_DIR)/build
INCLUDE_DIR = $(BASE_DIR)/include
TOOL_DIR    = $(BASE_DIR)/scripts/tools
IMAGES_DIR  = $(BASE_DIR)/data/images
MCONFIG_DIR = $(BASE_DIR)/scripts/config
SYSDEP_DIR  = $(BASE_DIR)/scripts/sysdep
EN_SHARED   = --enable-shared
EN_STATIC   = --enable-static

ifdef CONFIG_PLATFORM_DSLINUX
# here is the MicroWindows source code
MW_PREFIX      := $(BASE_DIR)/../microwin/src
# here is the installation root directory
INSTALL_PREFIX := $(BASE_DIR)/../../romfs/usr/pixil/
# we don't have static linkage here
EN_SHARED	= --disable-shared
CXXFLAGS := $(CXXFLAGS) -fno-rtti -fno-exceptions
endif

# The base directory for installing
ROOT_DIR = $(strip $(subst ",, $(INSTALL_PREFIX)))
TARGET_DIR = $(strip $(subst ",, $(TARGET_PREFIX)))

# The basic PDA install dir

ifdef CONFIG_PLATFORM_X86DEMO
INSTALL_DIR = $(ROOT_DIR)/pda
else
INSTALL_DIR=$(ROOT_DIR)
endif

MAKEFILES = $(BASE_DIR)/config

# The filenames for the PAR database files

PAR_CONFIG=$(INSTALL_DIR)/share/par/defaults.xml
PAR_DB=$(INSTALL_DIR)/share/par/pixil.db

# The cross compiler 
TARGET_CROSS=$(subst ",, $(strip $(CROSS_COMPILER_PREFIX)))

# The system building the kernel (we're going to assume)
# Intel until somebody proves us wrong

BUILD_SYS=i386-linux

ifeq ($(COMPILE_CROSS),y)
ifeq ($(CONFIG_TARGET_ARM),y)
SYS=arm-linux
endif
ifeq ($(CONFIG_TARGET_I386_UCLIBC),y)
SYS=i386-uclibc-linux
endif
ifeq ($(CONFIG_PLATFORM_DSLINUX),y)
SYS=arm-linux-elf
endif
else
SYS=$(BUILD_SYS)
endif

export TARGET_CROSS BASE_DIR STAGE_DIR INCLUDE_DIR PAR_DB PAR_CONFIG TARGET_DIR
export TOOL_DIR INSTALL_DIR ROOT_DIR SYS BUILD_SYS EN_SHARED EN_STATIC MW_PREFIX COMPILE_CROSS

# Set up the list of directories based on the configuration
ifdef SUBDIRS
DIRS-y=$(SUBDIRS)
else
DIRS-y=libs sys apps packages
DIRS-$(CONFIG_PIXILDT)+=pixilDT
endif

subdir-build = $(patsubst %,_subdir_%,$(DIRS-y))
subdir-clean = $(patsubst %,_clean_%,$(DIRS-y))
subdir-install = $(patsubst %,_install_%,$(DIRS-y))

# This avoids stripping the debugging information when we choose debugging 

STRIP_TARGET=

ifneq ($(CONFIG_DEBUG),y)
STRIP_TARGET=dostrip
endif

##### Toplevel targets #####

.PHONY: all install clean distclean
all: local-build
install: local-install 
clean: local-clean

distclean: local-clean packages-clean
	@ rm -rf config

# uCLinux builds: add PIXIL to the rom file system
.PHONY: romfs
romfs:	install

##### Local targets #####

.PHONY: local-build local-install local-clean dostrip
local-build: $(STAGE_DIR) $(INCLUDE_DIR)/pixil_config.h $(subdir-build) platform-build

local-install: $(INSTALL_DIR) $(subdir-install) $(STRIP_TARGET) platform-install
	@ mkdir -p $(INSTALL_DIR)/share/images
	@ cp -r $(IMAGES_DIR)/* $(INSTALL_DIR)/share/images
	@ rm -rf $(INSTALL_DIR)/share/images/.svn
 
local-clean:  $(subdir-clean) platform-clean
	@ rm -rf $(STAGE_DIR) $(INCLUDE_DIR)/pixil_config.h
	@ make -C scripts/config/ clean
 
dostrip:
	@ echo "Stripping binaries..."

	@ for fo in `find $(INSTALL_DIR) -type f -perm +111`; do \
		if file $$fo | grep "ELF" > /dev/null; then \
		$(TARGET_CROSS)strip $$fo; \
		fi; \
	done

.PHONY: $(subdir-build) $(subdir-clean) $(subdir-install)
$(subdir-build): dummy
	@ $(MAKE) -C $(patsubst _subdir_%,%,$@)

$(subdir-clean): dummy
	@ $(MAKE) -C $(patsubst _clean_%,%,$@) clean

$(subdir-install): dummy
	@ $(MAKE) -C $(patsubst _install_%,%,$@) install

.PHONY: platform-build platform-clean platform-install packages-clean
platform-build:
	@ $(MAKE) -C scripts/platforms/ 

platform-clean:
	@ $(MAKE) -C scripts/platforms/ clean

platform-install: 
	@ $(MAKE) -C scripts/platforms/ install

packages:
	@ $(MAKE) -C packages

packages-clean:
	@ $(MAKE) -C packages packages-clean

$(STAGE_DIR):
	@ mkdir -p $(STAGE_DIR)/lib

$(INSTALL_DIR):
	@ mkdir -p $(INSTALL_DIR)/lib
	@ mkdir -p $(INSTALL_DIR)/bin
	@ mkdir -p $(INSTALL_DIR)/share
	@ mkdir -p $(INSTALL_DIR)/share/data

$(INCLUDE_DIR)/pixil_config.h: $(BASE_DIR)/config
	@ perl $(TOOL_DIR)/config.pl $(BASE_DIR)/config \
	> $(INCLUDE_DIR)/pixil_config.h	

# configuration
# ------------------------------

$(MCONFIG_DIR)/conf:
	$(MAKE) -C $(MCONFIG_DIR) conf

$(MCONFIG_DIR)/mconf:
	$(MAKE) -C $(MCONFIG_DIR) conf mconf

menuconfig: $(BASE_DIR)/config $(MCONFIG_DIR)/mconf
	@$(MCONFIG_DIR)/mconf $(SYSDEP_DIR)/Config.in

textconfig: $(BASE_DIR)/config $(MCONFIG_DIR)/conf
	@$(MCONFIG_DIR)/conf $(SYSDEP_DIR)/Config.in

oldconfig: $(MCONFIG_DIR)/conf
	@$(MCONFIG_DIR)/conf -o $(SYSDEP_DIR)/Config.in

defconfig: $(BASE_DIR)/config 

$(BASE_DIR)/config:
	@ cp $(SYSDEP_DIR)/defconfig $(BASE_DIR)/config

.PHONY: defconfig_ipaq defconfig_zaurus defconfig_tuxscreen defconfig_dslinux
defconfig_ipaq:
	@ cp $(SYSDEP_DIR)/defconfig.ipaq $(BASE_DIR)/config

defconfig_zaurus:
	@ cp $(SYSDEP_DIR)/defconfig.zaurus $(BASE_DIR)/config

defconfig_tuxscreen:
	@ cp $(SYSDEP_DIR)/defconfig.tuxscreen $(BASE_DIR)/config

defconfig_dslinux:
	@ cp $(SYSDEP_DIR)/defconfig.dslinux $(BASE_DIR)/config

dummy:

.PHONY: menuconfig textconfig oldconfig defconfig dummy packages 
