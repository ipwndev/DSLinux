# apps/Makefile

target-y=

target-$(CONFIG_PIM_ADDRESS) += address
target-$(CONFIG_PIM_NOTEPAD) += notepad
target-$(CONFIG_PIM_TODO) += todo
target-$(CONFIG_PIM_SCHEDULE) += schedule

target-$(CONFIG_APP_ALARM) += alarm
target-$(CONFIG_APP_CALC) += calc
target-$(CONFIG_APP_NETCONFIG) += netconfig
target-$(CONFIG_APP_MP3) += mp3
target-$(CONFIG_APP_EMAIL) += mail
target-$(CONFIG_APP_SYSCONFIG) += sysconfig
target-$(CONFIG_APP_SYNC) += sync
target-$(CONFIG_APP_FIND) += find

# This will ensure that we only build one target at a time
DIRS=$(sort $(target-y))

subdir-build = $(patsubst %,_subdir_%,$(DIRS))
subdir-clean = $(patsubst %,_clean_%,$(DIRS))
subdir-install = $(patsubst %,_install_%,$(DIRS))

all: $(subdir-build)
clean: $(subdir-clean)
install: $(subdir-install)

$(subdir-build): dummy
	@ $(MAKE) -C $(patsubst _subdir_%,%,$@)

$(subdir-clean): dummy
	@ $(MAKE) -C $(patsubst _clean_%,%,$@) clean

$(subdir-install): dummy
	@ $(MAKE) -C $(patsubst _install_%,%,$@) install

dummy:

