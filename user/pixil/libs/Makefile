# libs/Makefile
# This will build all of the nessesary libraries

# Set the compiler targets for FLNX configure

#export CC=$(TARGET_CROSS)gcc
#export CXX=$(TARGET_CROSS)g++
#export AR=$(TARGET_CROSS)ar
#export NM=$(TARGET_CROSS)nm
#export RANLIB=$(TARGET_CROSS)ranlib
#export LD=$(TARGET_CROSS)ld

# FLNX and FLEK are always built

local-target-y=flnx
target-y=flek/src

target-$(CONFIG_PAR) += xml

target-$(CONFIG_PIM_ADDRESS) += pim
target-$(CONFIG_PIM_NOTEPAD) += pim
target-$(CONFIG_PIM_TODO) += pim
target-$(CONFIG_PIM_SCHEDULE) += pim

target-$(CONFIG_APP_ALARM) += pim
target-$(CONFIG_APP_CALC) += pim
target-$(CONFIG_APP_MAIL) += pim
target-$(CONFIG_APP_SYSCONFIG) += pixlib pim
target-$(CONFIG_SYNC) += pim sync

ifeq ($(CONFIG_NANOX),y)
target-$(CONFIG_APP_ALARM) += wm
target-$(CONFIG_PIXILWM) += pixlib wm xml
target-$(CONFIG_APP_NETCONFIG) += pixlib wm
target-$(CONFIG_NANOX_PIXCAL) += pixlib
endif

# This will ensure that we only build one target at a time
DIRS=$(sort $(target-y))
LOCAL=$(sort $(local-target-y))

subdir-build = $(patsubst %,_subdir_%,$(DIRS))
subdir-clean = $(patsubst %,_clean_%,$(DIRS))
subdir-install = $(patsubst %,_install_%,$(DIRS))

local-build = $(patsubst %,_build_%,$(LOCAL))
local-clean = $(patsubst %,_clean_%,$(LOCAL))
local-install = $(patsubst %,_install_%,$(LOCAL))

all: $(subdir-build) $(local-build)
clean: $(subdir-clean) $(local-clean)
install: $(subdir-install) $(local-install)

$(subdir-build): _build_flnx dummy
	@ $(MAKE) -C $(patsubst _subdir_%,%,$@)

$(subdir-clean): dummy
	@ $(MAKE) -C $(patsubst _clean_%,%,$@) clean

$(subdir-install): dummy
	@ $(MAKE) -C $(patsubst _install_%,%,$@) install


###### Local targets #######

$(BASE_DIR)/libs/flnx/.configured: 
	(cd $(BASE_DIR)/libs/flnx; ./configure \
	--host=$(SYS) \
        --build=$(BUILD_SYS) \
	$(EN_SHARED) \
	$(EN_STATIC) \
	--with-microwin=$(strip $(subst ",, $(MW_PREFIX))))
	touch $(BASE_DIR)/libs/flnx/.configured

#	make -C $(BASE_DIR)/libs/flnx/src -e 
_build_flnx: $(BASE_DIR)/libs/flnx/.configured
	make -C $(BASE_DIR)/libs/flnx/src

	rm -f $(STAGE_DIR)/lib/libfltk.a $(STAGE_DIR)/lib/libfltk.so $(STAGE_DIR)/lib/libfltk.so.1
	ln -s $(BASE_DIR)/libs/flnx/lib/libfltk.a $(STAGE_DIR)/lib
	ln -s $(BASE_DIR)/libs/flnx/lib/libfltk.so $(STAGE_DIR)/lib
	ln -s $(BASE_DIR)/libs/flnx/lib/libfltk.so $(STAGE_DIR)/lib/libfltk.so.1

ifeq ($(EN_SHARED),--enable-shared)
INSTALL_FLNX = install_flnx
endif
_install_flnx: $(INSTALL_FLNX)

install_flnx: dummy
	cp flnx/lib/libfltk.so $(INSTALL_DIR)/lib/
	rm -f $(INSTALL_DIR)/lib/libfltk.so.1
	ln -s $(INSTALL_DIR)/lib/libfltk.so $(INSTALL_DIR)/lib/libfltk.so.1

_clean_flnx: dummy
	if [ -f $(BASE_DIR)/libs/flnx/makeinclude ]; then \
		make -C $(BASE_DIR)/libs/flnx distclean; \
	fi
	rm -f $(BASE_DIR)/libs/flnx/.configured

dummy:
