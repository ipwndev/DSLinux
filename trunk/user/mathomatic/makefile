# Makefile for compiling, testing, and installing Mathomatic under any UNIX like OS.
# Currently uses gcc only options, just remove them for other C compilers.

VERSION		= `cat VERSION`
CFLAGS		+= -Wunused -Wshadow -DUNIX -DVERSION=\"$(VERSION)\"
#LDFLAGS		+= -s
LIBS		+= -lm

#CFLAGS		+= $(READLINE:1=-DREADLINE)
#LIBS		+= $(READLINE:1=-lreadline -lncurses)

prefix		?= /usr/local
bindir		?= $(prefix)/bin
mandir		?= $(prefix)/man
docdir		?= $(prefix)/share/doc

AOUT		= mathomatic
TARGETS		= $(AOUT) doc/manpage.html
OBJECTS		= main.o globals.o am.o solve.o help.o parse.o cmds.o simplify.o \
		  factor.o super.o unfactor.o poly.o diff.o integrate.o \
		  complex.o complex_lib.o list.o gcd.o factor_int.o
MAN1		= mathomatic.1
DOCS		= COPYING README.txt changes.txt doc/*

all: $(TARGETS)
	@cd lib && make
	@echo Make completed.

check: test

test: $(TARGETS)
	cd tests && time ../$(AOUT) -t all >test.out && diff -u all.out test.out
	@rm tests/test.out
	@echo All tests passed.

baseline: $(TARGETS)
	cd tests && time ../$(AOUT) -t all >all.out
	@rm -f tests/test.out

$(OBJECTS): includes.h am.h externs.h complex.h proto.h VERSION

$(AOUT): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $(AOUT)

doc/manpage.html: mathomatic.1
	groff -Thtml -man mathomatic.1 >doc/manpage.html

install:
	install -d $(bindir)
	install -d $(mandir)/man1
	install -d $(docdir)/mathomatic
	install -d $(docdir)/mathomatic/tests
	install -d $(docdir)/mathomatic/fact
	install -m 0755 $(AOUT) $(bindir)
	install -m 0644 $(MAN1) $(mandir)/man1
	install -m 0644 $(DOCS) $(docdir)/mathomatic
	install -m 0644 tests/* $(docdir)/mathomatic/tests
	install -m 0644 fact/* $(docdir)/mathomatic/fact
	@echo Install completed.

uninstall:
	rm -f $(bindir)/$(AOUT)
	cd $(mandir)/man1 && rm -f $(MAN1)
	rm -rf $(docdir)/mathomatic
	@echo Uninstall completed.

clean:
	rm -f *.o
	rm -f lib/*.o

romfs:
	$(ROMFSINST) /bin/mathomatic
