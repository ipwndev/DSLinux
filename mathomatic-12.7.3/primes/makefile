# Makefile for compiling and installing the Mathomatic Prime Number Tools under Linux and Unix.

CFLAGS		+= -O2 -Wall
LDFLAGS		+= -s -lm

prefix		?= /usr/local
bindir		?= $(prefix)/bin
mandir		?= $(prefix)/man

TARGETS		= matho-primes matho-pascal matho-sumsq
MANPAGES1	= matho-primes.1 matho-pascal.1 matho-sumsq.1

all: $(TARGETS)
	@echo Make completed.

matho-sumsq: matho-sumsq.o lsqrt.o

check test: $(TARGETS)
	time ./matho-primes 1000000000000000000 1000000000000300000 twin >test.out && diff twins.out test.out
	@rm test.out
	@echo Test passed.

install:
	install -d $(bindir)
	install -d $(mandir)/man1
	install -m 0755 $(TARGETS) $(bindir)
	install -m 0644 $(MANPAGES1) $(mandir)/man1
	@echo Install completed.

uninstall:
	cd $(bindir) && rm -f $(TARGETS)
	cd $(mandir)/man1 && rm -f $(MANPAGES1)

clean:
	rm -f *.o
	rm -f $(TARGETS)
