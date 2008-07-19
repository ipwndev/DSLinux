# apps/mp3/frontend/Makefile

TARGET_CXX=pixil-splay

OBJS=   mp3_button.o mp3utils.o fltksplay.o fspl_panel.o \
	comm.o xplay.o fft.o Mp3_Browser.o Mp3_Node.o Fl_Eq.o

LIBS=-L ../mpegsound -lmpeg -lpixil-pim

CFLAGS ?= 

CFLAGS += -DPIXIL

ifeq ($(CONFIG_SYNC),y)
LIBS+=-lpixil-sync
endif

ifeq ($(CONFIG_PAR),y)
LIBS+=-lpar
endif

LIBS+=-lflek
LIBS+=-lfltk

ifeq ($(CONFIG_NANOX),y)
LIBS+=-lnano-X 
endif

ifeq ($(CONFIG_X11),y)
LIBS+=-lX11
endif

ifeq ($(CONFIG_COLOSSEUM),y)
LIBS+=-lipc
endif

ifeq ($(CONFIG_MP3_USE_PTHREAD),y)
CFLAGS += -DPTHREADEDMPEG
LIBS+=-lpthread
endif

INCLUDES=-I. -I../mpegsound/

include $(BASE_DIR)/Rules.make

