# apps/sysconfig/Makefile

DIRS-y= src tcpip

subdir-build = $(patsubst %,_subdir_%,$(DIRS-y))
subdir-clean = $(patsubst %,_clean_%,$(DIRS-y))
subdir-install = $(patsubst %,_install_%,$(DIRS-y))

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

