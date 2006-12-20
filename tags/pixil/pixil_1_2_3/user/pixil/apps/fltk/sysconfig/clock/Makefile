# apps/sysconfig/about/Makefile

TARGET_SO=clock.so
INSTALL_SODIR=$(INSTALL_DIR)/share/sysconfig

SRC=${shell ls *.cxx} 
OBJS=${SRC:.cxx=.o}

INCLUDES=-I../include

CFLAGS ?= 
CFLAGS += -DPIXIL

include $(BASE_DIR)/Rules.make

