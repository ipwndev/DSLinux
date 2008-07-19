#games/snake/Makefile

TARGET=nxsnake
OBJS=nxsnake.o 
LIBS+=-lnano-X

ifeq ($(CONFIG_PAR),y)
LIBS+=-lpar
endif

INSTALL_EXTRAS=inst-snake

include $(BASE_DIR)/Rules.make

inst-snake:
	mkdir -p $(INSTALL_DIR)/share/images/
	cp snake.xpm $(INSTALL_DIR)/share/images
