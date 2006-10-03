# games/Makefile

target-y=
target-$(CONFIG_GAMES_MINESWEEP) += minesweep
target-$(CONFIG_GAMES_SNAKE) += snake
target-$(CONFIG_GAMES_NXBILL) += nxbill

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

