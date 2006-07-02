# tinyirc makefile
# by Nathan Laredo
#
# I don't wish to assert any rights (copyright) over this makefile
# but please give me credit if you use my code.
#
SERVER = irc.efnet.org
PORT = 6667
#
all: dslinux

debug:
	$(MAKE) tinyirc CFLAGS=-g LDFLAGS=-g CC=gcc LIBS=-ltermcap

generic:
	$(MAKE) tinyirc CFLAGS=-O LDFLAGS=-s LIBS=-ltermcap

aix:
	$(MAKE) tinyirccv CFLAGS="-O -D_AIX_" \
		LDFLAGS=-s LIBS=-lcurses CC=bsdcc

posix:
	$(MAKE) tinyirc CFLAGS="-O2 -m486 -DPOSIX" LDFLAGS="-s" LIBS=-ltermcap

gnu:
	$(MAKE) tinyirc CFLAGS="-O2 -pipe -DPOSIX -Wall -Wunused -Wformat" \
		LDFLAGS=-s LIBS=-ltermcap CC=gcc

hpux:
	$(MAKE) tinyirccv LDFLAGS=-s LIBS=-lcurses

ntest:
	$(MAKE) tinyirccv CFLAGS="-O -I/usr/include/ncurses -DPOSIX" \
		LDFLAGS="-L/usr/local/lib" LIBS=-lncurses CC=gcc

ctest:
	$(MAKE) tinyirccv CFLAGS=-O LDFLAGS=-s LIBS=-lcurses

dslinux:
	$(MAKE) tinyirc \
		CFLAGS="$(CFLAGS) -I$(ROOTDIR)/lib/libncurses/include" \
		LIBS="-L$(ROOTDIR)/lib/libncurses -lncurses"

DEFINES = -DDEFAULTSERVER=\"$(SERVER)\" -DDEFAULTPORT=$(PORT) -DCURSES -DPOSIX

tinyirc: tinyirc.o
	$(CC) $(LDFLAGS) -o tinyirc tinyirc.o $(LIBS)

tinyirccv: tinyirccv.o
	$(CC) $(LDFLAGS) -o tinyirc tinyirccv.o $(LIBS)

tinyirc.o: tinyirc.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) -c tinyirc.c -o tinyirc.o

tinyirccv.o: tinyirccv.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) -c tinyirccv.c -o tinyirccv.o

romfs:
	$(ROMFSINST) /bin/tinyirc

clean:
	rm -f tinyirc tinyirc.gdb tinyirc.o

#EOF
