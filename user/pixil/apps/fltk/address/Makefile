# apps/address/Makefile

TARGET_CXX=address

SRC=${shell ls *.cxx} 
OBJS=${SRC:.cxx=.o}

LIBS=-lpixil-pim

ifeq ($(CONFIG_SYNC),y)
LIBS+=-lpixil-sync
endif

ifeq ($(CONFIG_PAR),y)
LIBS+=-lpar
endif

LIBS+=-lflek
LIBS+=-lfltk

ifeq ($(CONFIG_NANOX),y)
LIBS+=-lnano-X 
endif

ifeq ($(CONFIG_X11),y)
LIBS+=-lX11
endif

ifeq ($(CONFIG_COLOSSEUM),y)
LIBS+=-lipc
endif

include $(BASE_DIR)/Rules.make

