# libs/pim/Makefile

LIB_STATIC=libpixil-pim.a
LIB_SHARED=libpixil-pim.so

CSRC=${shell ls *.c} 
CXXSRC=${shell ls *.cxx}

OBJS=${CSRC:.c=.o}
OBJS+=${CXXSRC:.cxx=.o}

CFLAGS ?= 
CFLAGS += -DPIXIL
include $(BASE_DIR)/Rules.make



