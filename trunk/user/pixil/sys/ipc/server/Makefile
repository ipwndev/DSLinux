#ipc/server/Makefile

TARGET=clserver
INSTALL_BINDIR=$(INSTALL_DIR)/sbin

SRC=${shell ls *.c} 
OBJS=${SRC:.c=.o}

LIBS=

ifeq ($(CONFIG_PAR),y)
LIBS+=-lpar
endif

include $(BASE_DIR)/Rules.make

