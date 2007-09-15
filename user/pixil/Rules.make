####
# Pixil Makefile
# Copyright 2002, Century Software
#
# Written by:  Jordan Crouse
# Released under the GPL (see LICENSE) for details
####

# stack size for uClinux
ifeq ($(STACK_SIZE),)
STACK_SIZE = 16384
endif

# Compiler information

# Get rid of the annoying quotes on the cross compiler

ifeq ($(BUILD_NATIVE),y)
CC=gcc
CXX=g++
AR=ar
RANLIB=ranlib
LD=ld

# Force X11 bulding if we are going native
CONFIG_NANOX=n
CONFIG_X11=y

else
AR=$(TARGET_CROSS)ar
RANLIB=$(TARGET_CROSS)ranlib
endif

# Determine if we are using a uclibc compiler or not
ifdef CONFIG_PLATFORM_DSLINUX
UCLIBC=uclibc
else
UCLIBC=$(findstring uclibc,$(CC))
endif

# Default paths

CURDIR=${shell pwd}
X11DIR?=/usr/X11R6
DEST_DIR?=$(TARGET_DIR)

# Dependency Paths

ifeq ($(CONFIG_NANOX),y)
MWDIR=$(strip $(subst ",, $(MW_PREFIX)))
endif

ifeq ($(BUILD_NATIVE),y)
ifeq ($(CONFIG_PIXILDT_FLTK),y)
FLTKDIR=$(strip $(subst ",, $(CONFIG_PIXILDT_FLTKDIR)))
else
FLTKDIR=$(BASE_DIR)/libs/flnx
endif
else
ifeq ($(CONFIG_FLNX),y)
FLTKDIR=$(BASE_DIR)/libs/flnx
endif
endif

ifeq ($(CONFIG_FLEK),y)
FLEKDIR=$(BASE_DIR)/libs/flek
endif

ifeq ($(CONFIG_VIEWML),y)
LIBWWW_CONFIG=$(strip $(subst ",, $(LIBWWW_PREFIX)))/bin/libwww-config
endif

# Installation directories

INSTALL_BINDIR ?= $(INSTALL_DIR)/bin
INSTALL_SODIR  ?= $(INSTALL_DIR)/plugins

# Compiler variables
LIBDIRS=

ifneq ($(BUILD_NATIVE),y)
LIBDIRS += -L$(STAGE_DIR)/lib
endif
 
INCLUDES += -I$(INCLUDE_DIR) 

CXXFLAGS += -Wall

CXXFLAGS += -DPIXIL

# If we are building with uclibc, then add in GNU_SOURCE
# This is hokey, but it works

ifeq ($(UCLIBC),uclibc)
CFLAGS += -D_GNU_SOURCE
endif

# Make supre that -fPIC is selected for cross compiled apps

#ifdef LIB_SHARED
#CFLAGS += -fPIC
#endif

# Compile with copious debug information

ifeq ($(CONFIG_DEBUG),y)
CXXFLAGS += -O0 -g -DDEBUG
endif

INCLUDES += -I$(FLTKDIR) -I$(FLEKDIR)

ifeq ($(CONFIG_NANOX),y)
CXXFLAGS += -DNANOX -DNANO_X
INCLUDES += -I$(MWDIR)/include
LIBDIRS += -L$(MWDIR)/lib
endif

ifeq ($(CONFIG_X11),y)
LIBDIRS += -L$(X11DIR)/lib/
endif

# Paranoia - it is up to the individual makefiles to define these
# if they don't then we zero them out for safety

PREBUILD_EXTRAS  ?= 
TARGET_EXTRAS    ?=
POSTBUILD_EXTRAS ?=
CLEAN_EXTRAS     ?=
INSTALL_EXTRAS   ?=

# Build targets

INST_STATIC=$(patsubst %, $(STAGE_DIR)/lib/%, $(LIB_STATIC))
INST_SHARED=$(patsubst %, $(STAGE_DIR)/lib/%, $(LIB_SHARED))

BUILD_LIBS :=
ifeq ($(EN_SHARED),--enable-shared)
BUILD_LIBS += $(LIB_SHARED)
endif
ifeq ($(EN_STATIC),--enable-static)
BUILD_LIBS += $(LIB_STATIC)
endif

BUILD_NATIVE_LIBS := $(NATIVE_LIB_STATIC) $(NATIVE_LIB_SHARED)
BUILD_BINS        := $(TARGET) $(TARGET_CXX)
BUILD_SO          := $(TARGET_SO)

BUILD_TARGETS := $(BUILD_LIBS) $(BUILD_BINS) $(BUILD_SO) $(BUILD_NATIVE_LIBS)

build-all: $(PREBUILD_EXTRAS) $(BUILD_TARGETS) $(TARGET_EXTRAS) $(POSTBUILD_EXTRAS)

clean: $(CLEAN_EXTRAS)
	@ rm -rf $(OBJS) $(NATIVE_OBJS) *.o core $(BUILD_LIBS) \
	$(BUILD_BINS) $(BUILD_SO) $(BUILD_NATIVE_LIBS)

	@ rm -rf $(PAR_CONFIG) $(LOCAL_DB)

INSTALL_TARGETS = install-bin $(INSTALL_EXTRAS)
ifeq ($(EN_SHARED),--enable-shared)
INSTALL_TARGETS += install-libs install-so
endif
install: $(INSTALL_TARGETS)

install-libs: $(INSTALL_DIR)/lib
	@ if [ -n "$(LIB_SHARED)" ]; then \
            cp $(LIB_SHARED) $(INSTALL_DIR)/lib/; \
         fi

install-bin: $(INSTALL_BINDIR)
	@ if [ -n "$(strip $(BUILD_BINS))" ]; then \
	  cp $(BUILD_BINS) $(INSTALL_BINDIR); \
	fi

install-so: $(INSTALL_SODIR)
	@ if [ -n "$(strip $(BUILD_SO))" ]; then \
	  cp $(BUILD_SO) $(INSTALL_SODIR); \
	fi

$(TARGET): $(OBJS)
	$(CC) $(BUILD_CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBDIRS) $(LDLIBS) $(LIBS) 
	$(TARGET_CROSS)flthdr -s $(STACK_SIZE) $@

$(TARGET_CXX): $(OBJS)
	$(CXX) -o $@ $(CRTBEGIN) $(BUILD_CFLAGS) $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LDLIBS) $(LIBS) $(CXXLIBS) -lm  $(CRTEND)
	$(TARGET_CROSS)flthdr -s $(STACK_SIZE) $@

$(TARGET_SO): $(OBJS)
	$(CC) -shared -o $@ $(OBJS)

$(LIB_STATIC): $(OBJS)
	$(AR) rc $@ $(OBJS)
	$(RANLIB) $@
	@ if [ ! -h $(STAGE_DIR)/lib/`basename $(LIB_STATIC)` ]; then \
		mkdir -p $(STAGE_DIR)/lib; \
		ln -s $(CURDIR)/$(LIB_STATIC) $(STAGE_DIR)/lib/`basename $(LIB_STATIC)`; \
	fi

$(LIB_SHARED): $(OBJS)
	$(CC) -Wl,-soname,`basename $@` -shared -o $@ $(OBJS)
	@ if [ ! -h $(STAGE_DIR)/lib/`basename $@` ]; then \
		mkdir -p $(STAGE_DIR)/lib; \
		ln -s $(CURDIR)/$(LIB_SHARED) $(STAGE_DIR)/lib/`basename $(LIB_SHARED)`; \
	fi

$(NATIVE_LIB_STATIC): $(NATIVE_OBJS)
	ar rc $@ $(NATIVE_OBJS)
	ranlib $@
	@ if [ ! -h $(STAGE_DIR)/lib/native/`basename $(NATIVE_LIB_STATIC)` ]; then \
		mkdir -p $(STAGE_DIR)/lib/native; \
		ln -s $(CURDIR)/$(NATIVE_LIB_STATIC) $(STAGE_DIR)/lib/native/`basename $(NATIVE_LIB_STATIC)`; \
	fi

$(NATIVE_LIB_SHARED): $(NATIVE_OBJS) 
	gcc -Wl,-soname,$@ -shared -o $@ $(NATIVE_OBJS)
	@ if [ ! -h $(STAGE_DIR)/lib/native/`basename $(NATIVE_LIB_SHARED)` ]; then \
		mkdir -p $(STAGE_DIR)/lib/native; \
		ln -s $(CURDIR)/$(NATIVE_LIB_SHARED) $(STAGE_DIR)/lib/native/`basename $(NATIVE_LIB_SHARED)`; \
	fi

$(STAGE_DIR)/lib/ $(STAGE_DIR)/lib/native $(INSTALL_DIR)/lib $(INSTALL_SODIR) $(INSTALL_BINDIR): 
	@ mkdir -p $@

## PAR database targets

$(PAR_DB): $(LOCAL_DB)
	@ mkdir -p `dirname $(PAR_DB)`	
	@ cp $(LOCAL_DB) $(PAR_DB)

$(LOCAL_DB): $(PAR_CONFIG)
	@ rm -f $(LOCAL_DB)
	$(STAGE_DIR)/bin/native/xmlimport -i $(PAR_CONFIG) $(LOCAL_DB)

$(PAR_CONFIG): $(PAR_TEMPLATE)
	mkdir -p `dirname $(PAR_CONFIG)`
	cat $(PAR_TEMPLATE) | sed -e s%@prefix@%$(DEST_DIR)%g > $(PAR_CONFIG)

##### SUFFIX TARGETS ######

%.o: %.c 
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

%.o: %.cc
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

%.o: %.cxx 
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) -o $@ $<

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) -o $@ $<


## These handle multiple platform targets

platform-objs/%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

native-objs/%.o: %.c
	gcc -c $(NATIVE_CFLAGS) $(INCLUDES) -o $@ $<
