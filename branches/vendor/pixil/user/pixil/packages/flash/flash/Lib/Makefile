CC=	gcc
CXX=	g++
OPTIMIZE=-g -O3

# Comment out the following line if you use egcs :
CXXFLAGS=$(OPTIMIZE) $(ALLCFLAGS) $(CXXFLAGS_$(CC)) -UNDEBUG
CXXFLAGS_gcc=  -Wall -fno-rtti -fno-exceptions

ifeq ($(RGB555),Y)
CXXFLAGS += -DRGB555=1
endif

CCC=	$(CXX)
CCFLAGS= $(CXXFLAGS)

INCLUDES= adpcm.h        cxform.h       graphic.h      script.h       sqrt.h\
	bitmap.h       displaylist.h  matrix.h       shape.h        swf.h\
	button.h       flash.h        program.h      sound.h        text.h\
	character.h    font.h         rect.h         sprite.h	movie.h\
	mp3.h

SWFOBJS=\
	flash.o \
	character.o \
	shape.o \
	button.o \
	program.o \
	bitmap.o \
	displaylist.o \
	font.o \
	graphic.o \
	text.o \
	matrix.o \
	script.o \
	sound.o \
	sprite.o \
	movie.o \
	cxform.o \
	adpcm.o \
	mp3.o \
	sqrt.o \
	graphic16.o \
	graphic24.o \
	graphic32.o

#	bitstream.o
#	dump.o

all: libflash.a

libflash.a: $(SWFOBJS) 
	ar rcs $@ $(SWFOBJS)

$(SWFOBJS): $(INCLUDES)

clean:
	rm -f $(SWFOBJS) *~ libflash.a

.PHONY: plugin jpeg all
