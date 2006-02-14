#==========================================
#    Makefile: makefile for sl
#	Copyright 1993,1998 Toyoda Masashi 
#		(toyoda@is.titech.ac.jp)
#	Last Modified: 1998/ 7/22
#       Modify for Debian by Kenshi Muto <kmuto@debian.org>
#         1999/ 6/14
#==========================================

CC=cc
CFLAGS=-Wall -O # -DDEBUG
LDFLAGS= -lcurses # -ltermcap

PREFIX=$(DESTDIR)/usr
BINDIR=$(PREFIX)/bin
MANDIR=$(PREFIX)/man
INSTALL=install

all: sl sl-h

sl: sl.c sl.h
	$(CC) $(CFLAGS) -o sl sl.c $(LDFLAGS)

sl-h: sl-h.c sl-h.h header.h
	$(CC) $(CFLAGS) -o sl-h sl-h.c $(LDFLAGS)

clean::
	rm -f sl sl-h

distclean:: clean

install::
	$(INSTALL) -m 755 sl $(BINDIR)/
	$(INSTALL) -m 755 sl-h $(BINDIR)/
	$(INSTALL) -m 644 sl.6 $(MANDIR)/man6/
	$(INSTALL) -m 644 sl-h.6 $(MANDIR)/man6/
	$(INSTALL) -m 644 sl.6j $(MANDIR)/ja/man6/sl.6
	$(INSTALL) -m 644 sl.6de $(MANDIR)/de/man6/sl.6
	$(INSTALL) -m 644 sl-h.6de $(MANDIR)/de/man6/sl-h.6

