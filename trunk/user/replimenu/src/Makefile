#
# Makefile for 'replimenu'.
# Copyright (C) 2003 Michel Blomgren, michel AT zebra DOT ath DOT cx.
#

include ../configuration

SRC = replimenu.c parser.c readkey.c
LMOBJ = $(SRC:.c=.$(OBJEXT))

all:	clean build

.c.$(OBJEXT):
	$(CC) $(CFLAGS) -DVERSIONSTRING=$(VERSIONSTRING) -c $<

build:	$(LMOBJ)
	$(CC) $(LDFLAGS) -o $(EXECNAME) $?

clean:
	@for i in *.o *.a core $(EXECNAME); do \
		if [ -e $$i ]; then \
			echo "Removing $$i"; \
			rm -f $$i; \
		fi; \
	done
