#!/usr/bin/make
#
# This makefile and all associated files has been placed into
# the public domain by Michael Kennett (July 2005), and can be
# used freely by anybody for any purpose.

OWN=-o bin
GRP=-g operator

# Minix requires compilation flags for a clean compile
sudoku: sudoku.c
	if [ -x /usr/bin/uname -a `/usr/bin/uname` = Minix ] ; then \
	DEFS="-D_MINIX -D_POSIX_SOURCE" ; fi ; \
	$(CC) -o sudoku $$DEFS sudoku.c -lcurses

clean:
	rm -f sudoku

# Simple support for alternate systems
install: sudoku sudoku.6
	if [ -x /usr/bin/uname -a `/usr/bin/uname` = Minix ] ; then \
	install -s -S 8kw $(OWN) $(GRP) -m 755 sudoku /usr/bin/sudoku ; \
	else \
	install -s $(OWN) $(GRP) -m 755 sudoku /usr/bin/sudoku ; \
	fi
	install -d $(OWN) $(GRP) -m 755 /usr/lib/sudoku
	install $(OWN) $(GRP) -m 644 template /usr/lib/sudoku/template
	if [ -d /usr/man/man6 ] ; then \
	install $(OWN) $(GRP) -m 644 sudoku.6 /usr/man/man6/sudoku.6 ; \
	elif [ -d /usr/share/man/man6 ] ; then \
	install $(OWN) $(GRP) -m 644 sudoku.6 /usr/share/man/man6/sudoku.6 ; \
	else \
	echo Warning: Manual page not installed ; \
	fi

