# Makefile for hexedit

PRODUCT = hexedit
VERSION = 1.2.12

SHELL = /bin/sh

#CC = ucfront-gcc
CFLAGS += -I ../../lib/libncurses/include

LIBS =  -L../../lib/libncurses/lib -lncurses
DEFS = -DHAVE_CONFIG_H
INSTALL = /usr/bin/install -c

# Installation directories
prefix = /usr/local
exec_prefix = ${prefix}
mandir = ${prefix}/man
bindir = ${exec_prefix}/bin

INCL = hexedit.h
SRCS = hexedit.c display.c mark.c page.c file.c interact.c misc.c search.c
OBJS = $(SRCS:.c=.o)

.SUFFIXES: .c .o

.c.o:
	$(CC) $(DEFS) $(CFLAGS) -c $<

all: $(PRODUCT)

$(PRODUCT): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

romfs:
	$(ROMFSINST) /bin/$(PRODUCT)
	$(ROMFSINST) -m hexedit.1 /usr/share/man/hexedit 

clean:
	rm -rf *~ *.o core *.cache config.status config.log $(PRODUCT)

distclean: clean
	rm -f Makefile config.h

install: $(PRODUCT)
	$(INSTALL) -d -m 755 $(bindir)
	$(INSTALL) -s -m 755 $(PRODUCT) $(bindir)
	$(INSTALL) -d -m 755 $(mandir)/man1
	$(INSTALL) -m 644 $(PRODUCT).1 $(mandir)/man1
