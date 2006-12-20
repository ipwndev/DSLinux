#games/nxbill/Makefile

TARGET_CXX=nxbill

OBJS=	Bucket.o Cable.o Computer.o Game.o Horde.o Library.o Monster.o Network.o \
	Scorelist.o Spark.o NXPicture.o NXwidgets.o NX.o nanox-UI.o

LIBS+=-lnano-X
CFLAGS ?= 
CFLAGS += -DPDA -DXBILL_HOME="\"$(INSTALL_DIR)/share/nxbill/\""

INSTALL_EXTRAS=inst-nxbill

include $(BASE_DIR)/Rules.make

inst-nxbill:
	mkdir -p $(INSTALL_DIR)/share/nxbill
	cp pixmaps/*.xpm $(INSTALL_DIR)/share/nxbill

