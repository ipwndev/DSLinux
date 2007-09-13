#par/lib/Makefile

LIB_STATIC=platform-objs/libpar.a
LIB_SHARED=platform-objs/libpar.so

NATIVE_LIB_STATIC=native-objs/libpar.a

PREBUILD_EXTRAS = $(CURDIR)/native-objs $(CURDIR)/platform-objs

SRC  := ${shell ls *.c}
OBJS := $(SRC:%.c=platform-objs/%.o)
NATIVE_OBJS := $(SRC:%.c=native-objs/%.o)

INCLUDES=-I./include

CFLAGS ?= 
CFLAGS +=-DHAVE_MMAPBUG -DDEFPARDB="\"$(PAR_DB)\""
NATIVE_CFLAGS=-DHAVE_MMAPBUG -DDEFPARDB="\"$(PAR_DB)\""
CXXFLAGS +=-DHAVE_MMAPBUG -DDEFPARDB="\"$(PAR_DB)\""
NATIVE_CXXFLAGS=-DHAVE_MMAPBUG -DDEFPARDB="\"$(PAR_DB)\""

include $(BASE_DIR)/Rules.make

$(CURDIR)/platform-objs $(CURDIR)/native-objs:
	@ mkdir -p $@

clean:
	@ rm -rf $(CURDIR)/native-objs $(CURDIR)/platform-objs


