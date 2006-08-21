# Makefile for mpc
#
# Originally based from hexedit Makefile
#
# (by Tuna)
#

PRODUCT = mpc
VERSION = 0.11.2

SHELL = /bin/sh

#CC = ucfront-gcc
CFLAGS += -Os 

LIBS = 
DEFS = -DHAVE_CONFIG_H
INSTALL = /usr/bin/install -c

# Installation directories
prefix = /usr/local
exec_prefix = ${prefix}
mandir = ${prefix}/man
bindir = ${exec_prefix}/bin


all: 
	make -C src

romfs:
	make -C src romfs

clean:
	make -C src clean

#distclean: clean
#	rm -f Makefile config.h

install:
	make -C src install
