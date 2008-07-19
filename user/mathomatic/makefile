# Makefile for compiling, testing, and installing Mathomatic under any UNIX like OS.
# Currently uses gcc only options in CFLAGS, just remove them for other C compilers.

VERSION		= `cat VERSION`
CFLAGS		+= -Wuninitialized -Wunused -Wshadow -Wformat -Wparentheses -Wcast-align
CFLAGS		+= -O -DUNIX -DVERSION=\"$(VERSION)\"
#LDFLAGS		+= -s
LIBS		+= -lm

# "make READLINE=1" to include readline support:
CFLAGS		+= $(READLINE:1=-DREADLINE)
LIBS		+= $(READLINE:1=-lreadline -lncurses)

# Install directories:
prefix		?= /usr/local
bindir		?= $(prefix)/bin
mandir		?= $(prefix)/man
docdir		?= $(prefix)/share/doc

AOUT		= mathomatic # The name of the executable to create.
TARGETS		= $(AOUT)

OBJECTS		= main.o globals.o am.o solve.o help.o parse.o cmds.o simplify.o \
		  factor.o super.o unfactor.o poly.o diff.o integrate.o \
		  complex.o complex_lib.o list.o gcd.o factor_int.o

MAN1		= mathomatic.1
DOCS		= COPYING README.txt changes.txt mathomatic.ico

all: $(TARGETS)
	@echo Make completed.

check test: $(TARGETS)
	@echo Testing Mathomatic...
	@cd tests && time ../$(AOUT) -t all >test.out && diff -u all.out test.out
	@rm tests/test.out
	@echo All tests passed.

baseline: $(TARGETS)
	@cd tests && ../$(AOUT) -t all >all.out
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
	install -d $(docdir)/mathomatic/html
	install -d $(docdir)/mathomatic/tests
	install -d $(docdir)/mathomatic/factorial
	install -m 0755 $(AOUT) $(bindir)
	install -m 0644 $(MAN1) $(mandir)/man1
	install -m 0644 $(DOCS) $(docdir)/mathomatic
	install -m 0644 doc/* $(docdir)/mathomatic/html
	install -m 0644 tests/* $(docdir)/mathomatic/tests
	install -m 0644 factorial/* $(docdir)/mathomatic/factorial
	@echo Install completed.

uninstall:
	rm -f $(bindir)/$(AOUT)
	cd $(mandir)/man1 && rm -f $(MAN1)
	rm -rf $(docdir)/mathomatic
	@echo Uninstall completed.

clean:
	rm -f *.o *.a
	rm -f lib/*.o lib/*.a
	rm -f *.gdb
	rm -f $(AOUT)
	rm -f mathomatic_secure

romfs:
	$(ROMFSINST) /bin/mathomatic
	$(ROMFSINST) -m mathomatic.1 /usr/share/man/mathomatic

flush: clean
	rm -f $(AOUT)
	rm -f mathomatic_secure
