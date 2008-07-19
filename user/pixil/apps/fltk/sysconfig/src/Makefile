# apps/sysconfig/src

TARGET_CXX=sysconfig

SRC=${shell ls *.cxx} 
OBJS=${SRC:.cxx=.o}

LIBS= -lpixil -lpixil-pim

# Note - this is a bad bad thing

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

ifeq ($(EN_SHARED),--enable-shared)
LIBS+=-ldl
endif

BUILD_CFLAGS=-rdynamic 
INCLUDES=-I../include

include $(BASE_DIR)/Rules.make

