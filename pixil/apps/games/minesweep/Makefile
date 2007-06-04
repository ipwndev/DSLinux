#games/minesweep/Makefile

TARGET=minesweep
OBJS=minesweep.o
LIBS+=-lnano-X 

ifeq ($(CONFIG_PAR),y)
LIBS+=-lpar
endif

INSTALL_EXTRAS=inst-mine

include $(BASE_DIR)/Rules.make

inst-mine:
	mkdir -p $(INSTALL_DIR)/share/images/
	cp mine.xpm $(INSTALL_DIR)/share/images/

