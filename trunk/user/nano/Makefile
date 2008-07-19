# Makefile for nano
#
# Originally based on hexedit Makefile
#
# (by Tuna)

PRODUCT = nano
VERSION = 1.2.5

SHELL = /bin/sh

#CC = ucfront-gcc
CFLAGS += -Os -I../../lib/libncurses/include

LIBS =  -L../../lib/libncurses/lib -lncurses
DEFS = -DHAVE_CONFIG_H
INSTALL = /usr/bin/install -c

# Installation directories
prefix = /usr/local
exec_prefix = ${prefix}
mandir = ${prefix}/man
bindir = ${exec_prefix}/bin


INCL = nano.h proto.h 
SRCS = color.c  cut.c  files.c  global.c  move.c  nano.c  rcfile.c  search.c  utils.c  winio.c
OBJS = $(SRCS:.c=.o)

.SUFFIXES: .c .o

.c.o:
	$(CC) $(DEFS) $(CFLAGS) -c $<

all: $(PRODUCT)

$(PRODUCT): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

romfs:
	$(ROMFSINST) /bin/$(PRODUCT)
	$(ROMFSINST) -m nano.1 /usr/share/man/nano
	$(ROMFSINST) -m nanorc.5 /usr/share/man/nanorc

clean:
	rm -rf *~ *.o core *.cache config.status config.log $(PRODUCT) $(PRODUCT).gdb

install: $(PRODUCT)
	$(INSTALL) -d -m 755 $(bindir)
	$(INSTALL) -s -m 755 $(PRODUCT) $(bindir)
	$(INSTALL) -d -m 755 $(mandir)/man1
	$(INSTALL) -m 644 $(PRODUCT).1 $(mandir)/man1
