BIN		=	cnibbles
OBJ		=	main.o misc.o screen.o options.o highscore.o replay.o intro.o
LDFLAGS		+=	-lncurses 
#CC		=	gcc
#CFLAGS		=	-O0 -Wall -g
HEADERS 	=	misc.h config.h screen.h options.h highscore.h replay.h intro.h

all: $(BIN)

clean:
	rm -f $(BIN) $(OBJ) core *.gdb *.elf

static: $(BIN_S)

install:
	cp -f cnibbles /usr/local/bin/

romfs: $(BIN)
	$(ROMFSINST) /usr/games/$(BIN)

$(BIN): $(OBJ)
	$(CC) -o $(BIN) $(OBJ) $(LDFLAGS) $(CFLAGS)

misc.o: misc.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

main.o: main.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

screen.o: screen.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

options.o: options.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

highscore.o: highscore.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

replay.o: replay.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: all clean romfs
