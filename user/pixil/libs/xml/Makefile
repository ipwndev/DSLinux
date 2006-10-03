#libs/xml/Makefile 

LIB_STATIC=platform-objs/libxml.a
LIB_SHARED=platform-objs/libxml.so

NATIVE_LIB_STATIC=native-objs/libxml.a

PREBUILD_EXTRAS = $(CURDIR)/native-objs $(CURDIR)/platform-objs

SRC=${shell ls *.c}

OBJS := $(SRC:%.c=platform-objs/%.o)
NATIVE_OBJS := $(SRC:%.c=native-objs/%.o)

INCLUDES=-I./include

include $(BASE_DIR)/Rules.make

$(CURDIR)/platform-objs $(CURDIR)/native-objs:
	@ mkdir -p $@

clean:
	rm -rf $(CURDIR)/platform-objs $(CURDIR)/native-objs

