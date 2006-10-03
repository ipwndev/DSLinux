#nanox/scribble 

TARGET=nxscribble

OBJS=   nxscribble.o scribwidget.o widgets.o\
	hre_api.o li_recognizer.o bitvector.o matrix.o sc.o util.o

LIBS+=-lwm -lnano-X -lm

ifeq ($(CONFIG_COLOSSEUM),y)
LIBS+=-lipc
endif

ifeq ($(CONFIG_PAR),y)
LIBS+=-lpar
endif

INSTALL_EXTRAS=inst-scribble

include $(BASE_DIR)/Rules.make

inst-scribble:
	mkdir -p $(INSTALL_DIR)/share/scribble
	cp *.cl $(INSTALL_DIR)/share/scribble
