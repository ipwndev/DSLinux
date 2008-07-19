#nanox/wclock

# This determines which image to use - I don't think
# this should be added to the config, but I might be wrong

IMAGE=images/earth_2d.jpg

TARGET=wclock

OBJS=main.o city.o astro.o

LIBS+=-lnano-X -lm
INSTALL_EXTRAS=install-wclock

ifeq ($(CONFIG_PAR),y)
LIBS+=-lpar
endif

include $(BASE_DIR)/Rules.make

install-wclock:
	mkdir -p $(INSTALL_DIR)/share/worldclock

	cp data/city.list $(INSTALL_DIR)/share/worldclock
	cp $(IMAGE) $(INSTALL_DIR)/share/worldclock/earth.jpg
