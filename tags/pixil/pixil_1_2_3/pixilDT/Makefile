# apps/pixilDT/Makefile

PIXILDT_DIR=$(ROOT_DIR)/workstation
DEST_DIR?=$(ROOT_DIR)

export PIXILDT_DIR

DIRS =

ifndef CONFIG_PIXILDT_FLTK
DIRS += fltk 
endif

DIRS += flek NxDb

ifeq ($(CONFIG_SYNC),y)
DIRS += sync syncapp
endif

DIRS += src

subdir-build = $(patsubst %,_subdir_%,$(DIRS))
subdir-clean = $(patsubst %,_clean_%,$(DIRS))
subdir-install = $(patsubst %,_install_%,$(DIRS))

all: $(subdir-build)
clean: $(subdir-clean)
install: $(subdir-install) $(PIXILDT_DIR)/PixilDT.ini $(DEST_DIR)/run_desktop.sh

$(subdir-build): dummy
	@ $(MAKE) -C $(patsubst _subdir_%,%,$@)

$(subdir-clean): dummy
	@ $(MAKE) -C $(patsubst _clean_%,%,$@) clean

$(subdir-install):
	@ $(MAKE) -C $(patsubst _install_%,%,$@) install

$(PIXILDT_DIR): 
	@ mkdir -p $(PIXILDT_DIR)
	@ mkdir -p $(PIXILDT_DIR)/bin

	@ mkdir -p $(PIXILDT_DIR)/data
	@ chmod 777 $(PIXILDT_DIR)/data

$(PIXILDT_DIR)/PixilDT.ini: $(PIXILDT_DIR)
	@ cat PixilDT.ini.templ | \
	sed -e s%Path=.*%Path=$(DEST_DIR)/workstation/data%g \
	> $(PIXILDT_DIR)/bin/PixilDT.ini

$(DEST_DIR)/run_desktop.sh:
	cat run_desktop.sh.in | \
	sed -e s%@prefix@%$(DEST_DIR)/workstation%g \
	> $(DEST_DIR)/run_desktop.sh 

	chmod uga+x $(DEST_DIR)/run_desktop.sh

dummy:

.PHONY: fltk flek NxDb src

