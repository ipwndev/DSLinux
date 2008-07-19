# apps/mp3/mpegsound/Makefile

LIB_STATIC=libmpeg.a

SRC=	soundinputstream.cc fileinput.cc httpinput.cc \
	soundplayer.cc rawplayer.cc rawtofile.cc \
	mpegtable.cc filter.cc filter_2.cc \
	mpegtoraw.cc mpeglayer1.cc mpeglayer2.cc \
	mpeglayer3.cc bitwindow.cc huffmantable.cc \
	wavetoraw.cc fileplayer.cc

OBJS=${SRC:.cc=.o}

CFLAGS ?= 
ifeq ($(CONFIG_MP3_USE_PTHREAD),y)
CFLAGS += -DPTHREADEDMPEG
endif

ifeq ($(COMPILE_CROSS),y)
CFLAGS += -fpermissive
endif

include $(BASE_DIR)/Rules.make



