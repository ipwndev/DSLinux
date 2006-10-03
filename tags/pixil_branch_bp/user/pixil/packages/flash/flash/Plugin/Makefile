#!make
################################################################################
# Copyright (c) 1996 Netscape Communications. All rights reserved.
################################################################################
#
# The output of the make process will be npflash.so
# Install this file either in
#	/usr/lib/netscape/plugins/
#	(or)
#	~/.netscape/plugins/
#	(or) in any convenient directory and point environment variable
#	     NPX_PLUGIN_PATH to point to the directory. It is advisable
#	     that the plugins (.so) are the only files in that directory.
#
# This makefile contains some of our defines for the compiler:
#
# XP_UNIX	This needs to get defined for npapi.h on unix platforms.
# PLUGIN_TRACE	Enable this define to get debug prints whenever the plugin
#		api gets control.
#          


PLUGIN_DEFINES= -DXP_UNIX
OPTIMIZE=-O
FLASH=../Lib

LIBCXX = $(LIBCXX_$(CXX))
# Solaris
LIBCXX_CC = -lCrun
# gcc
LIBCXX_g++ =

# Solaris (Sun compiler/linker)
SHARED=-G
# (Free|Net)BSD
#SHARED=-Wl,-Bshareable
# Linux
#SHARED=-shared

X11DIR=	/usr/X11R6
CFLAGS=  $(OPTIMIZE) $(PLUGIN_DEFINES) $(ALLCFLAGS) -I$(FLASH) -I$(X11DIR)/include
LDFLAGS=$(SHARED)
LIBS= $(FLASH)/libflash.a $(JPEG_LIB) $(Z_LIB) $(LIBCXX) -L$(X11DIR)/lib -lX11

SRC= plugin.c npunix.c
OBJ= plugin.o npunix.o

SHAREDTARGET=npflash.so

default all: $(SHAREDTARGET)

$(SHAREDTARGET): $(OBJ) $(FLASH)/libflash.a
	$(CXX) $(LDFLAGS) -o $(SHAREDTARGET) $(OBJ) $(LIBS)

plugin.o: $(FLASH)/flash.h

clean:
	$(RM) $(OBJ) $(SHAREDTARGET)
