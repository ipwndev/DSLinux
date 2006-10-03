# sys/Makefile
# This will build all of the nessesary libraries

target-y=

target-$(CONFIG_PAR) += par
target-$(CONFIG_COLOSSEUM) += ipc
target-$(CONFIG_PIXILWM) += pixilwm
target-$(CONFIG_SYNC) += syncagent

DIRS=$(target-y)

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
