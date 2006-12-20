#!/bin/make

# Uncomment to build for 555 format 16bpp system (565 default)
RGB555=Y

# Uncomment to build Nano-X version
NANOXDIR=/usr2/home/greg/net/microwin/src
GUIAPP=main_nx.c
GUIINC=$(NANOXDIR)/include
GUILIBS=-L$(NANOXDIR)/lib -lnano-X

# Uncomment to build X11 version
#X11DIR=/usr/X11R6
#GUIAPP=main_x11.c
#GUIINC=$(X11DIR)/include
#GUILIBS=-L$(X11DIR)/lib -lX11 -lXext

# Uncomment to compile using GNU C/C++
CC=		gcc
CXX=		g++

# Uncomment to compile with Sun-C/CC
#CC=		cc
#CXX=		CC

ALLCFLAGS=	$(SHAREDCFLAGS) $(AUDIO_DEFS) $(JPEG_INCL) $(Z_INCL)
#SHAREDCFLAGS=	$(SHAREDCFLAGS_$(CC))
#SHAREDCFLAGS_cc= -KPIC
#SHAREDCFLAGS_gcc= -fPIC

#OPTIMIZE=	-g
OPTIMIZE=	-g -O

#
#	JPEG
#

# Compile and use the "jpeg" sources from the flash-0.x distribution
#JPEG_SUBDIR=	Jpeg
#JPEG_INCL=	-I../Jpeg
#JPEG_LIB=	../Jpeg/libjpeg.a

# Use system jpeg library
JPEG_SUBDIR=
JPEG_INCL=
JPEG_LIB=	-ljpeg
#JPEG_INCL=	-I/opt/sfw/include/
#JPEG_LIB=	-L/opt/sfw/lib -R/opt/sfw/lib -ljpeg


#
#	ZLIB
#

# Compile and use the "zlib" sources from the flash-0.x distribution
#Z_SUBDIR=	Zlib
#Z_INCL=		-I../Zlib
#Z_LIB=		../Zlib/libz.a

# Use the system's zlib 
Z_SUBDIR=
Z_INCL=
Z_LIB=		-lz


#
#	AUDIO
#

AUDIO_DEFS=	-DOSS_SOUND
#AUDIO_DEFS=	-DSUN_SOUND
#AUDIO_DEFS=	-DNOSOUND

#SUBDIRS= $(JPEG_SUBDIR) $(Z_SUBDIR) Lib Player Plugin Kflash
SUBDIRS= $(JPEG_SUBDIR) $(Z_SUBDIR) Lib Player

all:
	for subdir in $(SUBDIRS); do \
		(cd $$subdir; \
		$(MAKE) CC="$(CC)" CXX="$(CXX)" ALLCFLAGS="$(ALLCFLAGS)" \
			OPTIMIZE="$(OPTIMIZE)" \
			RGB555="$(RGB555)" \
			GUIAPP="$(GUIAPP)" GUIINC="$(GUIINC)" GUILIBS="$(GUILIBS)"  \
			JPEG_LIB="$(JPEG_LIB)" Z_LIB="$(Z_LIB)"); \
	done

clean:
	for subdir in $(SUBDIRS); do \
		(cd $$subdir; $(MAKE) clean); \
	done
