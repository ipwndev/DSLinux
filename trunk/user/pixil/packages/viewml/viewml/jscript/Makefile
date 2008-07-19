YACC = bison -y
YFLAGS = -d -p yyjscript
LEX = flex
SOURCES = t1.c t2.c cpp2bison.cpp jstree.cpp jsexec.cpp builtin.cpp
OBJS = t1.o t2.o cpp2bison.o jstree.o jsexec.o builtin.o

INCLUDES=-I../src/fltk
OBJECTS = $(CPPFILES:.cxx=.o) $(CFILES:.c=.o)

all: grammarfiles $(OBJS)

grammarfiles: t1.y t2.l
	$(YACC) $(YFLAGS) t1.y $< && mv y.tab.c t1.c
	if test -f y.tab.h; then \
	if cmp -s y.tab.h t1.h; then rm -f y.tab.h; \
	else mv y.tab.h t1.h; fi \
	else :; fi
	$(LEX) -Pyyjscript t2.l && mv lex.yyjscript.c t2.c

.SUFFIXES:	.cpp .h .o

.cpp.o :
	$(CXX) $(INCLUDES) -I.. $(CXXFLAGS) -c -o $@ $<
.c.o :
	$(CC) -I.. $(INCLUDES)  $(CFLAGS) -c -o $@ $<

clean :
	-@ rm -f *.o *.do $(DSONAME) $(LIBRARY) $(CLEAN) core *~ ../include/*~ makedepend cmap
