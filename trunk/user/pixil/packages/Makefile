# Include the depends for the file
-include .depends

PKGCONF=pkg.conf
BUILD_DIR=${shell pwd}/build
SOURCE_DIR=${shell pwd}/sources

export BUILD_DIR SOURCE_DIR

subdir-build = $(patsubst %,_subdir_%,$(DIRS-y))
subdir-clean = $(patsubst %,_clean_%,$(DIRS-y))
subdir-install = $(patsubst %,_install_%,$(DIRS-y))

all: $(BUILD_DIR) $(subdir-build)

clean: $(subdir-clean) packages-clean
install: $(subdir-install)

packages-clean:
	@ rm -rf $(BUILD_DIR)
	@ rm -rf $(SOURCE_DIR)/*
	@ rm -rf .depends
	
$(subdir-build): dummy
	@ $(MAKE) -C $(patsubst _subdir_%,%,$@) all

$(subdir-clean): dummy
	@ $(MAKE) -C $(patsubst _clean_%,%,$@) clean

$(subdir-install): dummy
	@ $(MAKE) -C $(patsubst _install_%,%,$@) install

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.depends: $(PKGCONF)
	@ ./scripts/makedeps.sh $(PKGCONF) > .depends

dummy:

.PHONY: dummy all clean install
