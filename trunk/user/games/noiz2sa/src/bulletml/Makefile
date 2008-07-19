#CC = g++
INCLUDES = -I.
LIBS = tinyxml/tinyxml.o tinyxml/tinyxmlparser.o tinyxml/tinyxmlerror.o
#CXXFLAGS = -g -W -Wall -ansi -pedantic
#CXXFLAGS = -pg -g -W -Wall -ansi -pedantic
#CXXFLAGS = -I$(ROOTDIR)/lib/uClibc++/include -O2 -g -fomit-frame-pointer -fno-common -fno-builtin -Wall -mswp-byte-writes -DCONFIG_NDS_ROM8BIT -mcpu=arm946e-s -mfpu=fpe3 -DEMBED -D__PIC__ -fpic -msingle-pic-base -Dlinux -D__linux__ -Dunix -D__uClinux__ -I /home/gpf/dslinux/lib/uClibc++/include -fno-builtin -nostdinc++ -DGCC_HASCLASSVISIBILITY -fexceptions -Wno-deprecated

CFLAGS += 
OBJS = bulletmlparser-tinyxml.o bulletmlparser.o bulletmltree.o calc.o formula-variables.o bulletmlrunner.o bulletmlrunnerimpl.o

all: tinyxml libbulletml.a

tinyxml:
	$(MAKE) -C tinyxml

libbulletml.a: $(OBJS)
		#$(CXX) $(CXXFLAGS) -o $@ $(CRTBEGIN) $(LDFLAGS) $(OBJS) $(CXXLIBS) $(LDLIBS) $(CRTEND)
	$(AR) -r libbulletml.a *.o tinyxml/tiny*.o

clean:
	rm -f *.o *.a *.elf2flt
	$(MAKE) -C tinyxml clean

#calc.cpp: calc.yy
#	bison -y   calc.yy && mv y.tab.c calc.cpp

$(OBJS): %.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDES) -o $@ $<

doxy:
	doxygen

distdir:

