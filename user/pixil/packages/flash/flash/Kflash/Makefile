TARGET = kflash.kss

OBJECTS =  \
	kflash.o

COMMON = saver_common
SAVER_LIBS = $(COMMON)/libsvrcmn.a

KDEDIR = /usr
X11=/usr/X11R6

# Solaris
QTDIR = /usr/local/qt
QTINCL = -I$(QTDIR)/include
# Linux
#QTINCL = -I/usr/include/qt

CC = gcc
CXX = g++
CXXFLAGS = -O $(QTINCL) -I$(COMMON) -I../Lib
LD = g++
LDFLAGS = -s -L../Lib
LIBS = -lflash ../Zlib/libz.a ../Jpeg/libjpeg.a -lqt -lcrypt -lkdeui -lkdecore -lpam 

ifeq ($(EN_SHARED),--enable-shared)
LIBS+=-ldl
endif

AR = ar rc
RANLIB = ar s
MOC = moc

CCC = $(CXX)
CCFLAGS = $(CXXFLAGS) 


all: $(TARGET)

$(TARGET): $(OBJECTS) $(SAVER_LIBS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS) $(SAVER_LIBS)

$(COMMON)/libsvrcmn.a:
	make -C $(COMMON)

clean:
	-rm -f core $(OBJECTS) *.moc
	-rm -f $(TARGET)
	(cd $(COMMON) && make clean)

clean-all:
	make clean
	-rm -f *~ $(TARGET) $(SAVER_LIBS)

%.moc: %.h
	$(MOC) $*.h -o $*.moc

# dependencies
kflash.o: kflash.moc kflash.h
kflash.moc: kflash.h
