# libs/pim/Makefile

LIB_STATIC=libpixil-pim.a
ifeq ($(EN_SHARED),--enable-shared)
LIB_SHARED=libpixil-pim.so
else
LIB_SHARED=
endif

CSRC=${shell ls *.c} 
CXXSRC=${shell ls *.cxx}

OBJS=${CSRC:.c=.o}
OBJS+=${CXXSRC:.cxx=.o}

CFLAGS ?= 
CFLAGS += -DPIXIL
include $(BASE_DIR)/Rules.make



