# apps/address/Makefile

TARGET_CXX=netconfig

SRC=${shell ls *.cxx} 
OBJS=${SRC:.cxx=.o}

ifeq ($(COMPILE_CROSS),y)
CFLAGS ?= 
CFLAGS += -fpermissive
endif

LIBS=-lpixil-pim -lpixil

ifeq ($(CONFIG_PAR),y)
LIBS+=-lpar
endif

ifeq ($(CONFIG_SYNC),y)
LIBS+=-lpixil-sync
endif

LIBS+=-lflek
LIBS+=-lfltk

ifeq ($(CONFIG_NANOX),y)
LIBS+=-lwm -lnano-X 
endif

ifeq ($(CONFIG_X11),y)
LIBS+=-lX11
endif

ifeq ($(CONFIG_COLOSSEUM),y)
LIBS+=-lipc
endif

include $(BASE_DIR)/Rules.make

