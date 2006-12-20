# apps/address/Makefile

DEST_DIR?=$(ROOT_DIR)

# Indicate that we want to build this natively
BUILD_NATIVE=y

ifeq ($(CONFIG_PIXILDT_INTERNATIONAL),y)
GETTEXT=/usr/share/gettext
CFLAGS=-I$(GETTEXT)
endif

TARGET_CXX=PixilDT
INSTALL_BINDIR=$(PIXILDT_DIR)/bin

SRC=${shell ls *.cpp} 
OBJS=${SRC:.cpp=.o}

ifeq ($(CONFIG_SYNC),y)
INCLUDES=-I../sync/
endif

CFLAGS += -DDB_DIR="\"$(DEST_DIR/workstation/data)\""

LIBS= -L$(BASE_DIR)/pixilDT/flek -lflek

ifeq ($(CONFIG_PIXILDT_FLTK),y)
LIBS += -L$(FLTKDIR)/lib -lfltk
else
LIBS += -L$(BASE_DIR)/pixilDT/fltk -lfltk
endif

LIBS += ../NxDb/libNxDb.a 

ifeq ($(CONFIG_SYNC),y)
LIBS += ../sync/libpixil-sync-native.a
endif

LIBS += -lX11

include $(BASE_DIR)/Rules.make

