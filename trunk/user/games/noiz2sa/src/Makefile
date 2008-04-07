# Noiz2sa makefile(MinGW 2.0.0)
# $Id: Makefile,v 1.4 2003/08/10 09:00:05 kenta Exp $

NAME   = noiz2sa
O      = o
RM     = rm -f
#CC     = gcc
#CXX    = g++

PROG        = $(NAME)

DEFAULT_CFLAGS = -DSCREEN_DIVISOR=2 -I$(ROOTDIR)/include
LDLIBS       += -L$(ROOTDIR)/lib -Lbulletml -lSDL_mixer -lvorbisidec -lbulletml -lSDL -lpthread -lm $(CXXLIBS)

MORE_CFLAGS = 
#CXXFLAGS = -O2 -g -fomit-frame-pointer -fno-common -fno-builtin -Wall -mswp-byte-writes -DCONFIG_NDS_ROM8BIT -mcpu=arm946e-s -mfpu=fpe3 -DEMBED -D__PIC__ -fpic -msingle-pic-base -Dlinux -D__linux__ -Dunix -D__uClinux__ -I /home/gpf/dslinux/lib/uClibc++/include -fno-builtin -nostdinc++ -DGCC_HASCLASSVISIBILITY -fexceptions -Wno-deprecated

CFLAGS  += $(DEFAULT_CFLAGS) $(MORE_CFLAGS)
CPPFLAGS  +=  $(DEFAULT_CFLAGS) $(MORE_CFLAGS) -I./bulletml/

OBJS =	$(NAME).$(O) ship.$(O) shot.$(O) frag.$(O) bonus.$(O) \
	foe.$(O) foecommand.$(O) barragemanager.$(O) attractmanager.$(O) \
	background.$(O) letterrender.$(O) \
	screen.$(O) clrtbl.$(O) vector.$(O) degutil.$(O) rand.$(O) soundmanager.$(O)

$(PROG): $(OBJS) 
	$(CXX) $(CXXFLAGS) -o $@ $(CRTBEGIN) $(LDFLAGS) $(OBJS) $(CXXLIBS) $(LDLIBS) $(CRTEND) 

clean:
	$(RM) $(PROG) $(PROG).gdb *.$(O) 

