# Makefile for Pacman
 
LIBS =  -L$(ROOTDIR)/lib/libncurses/lib -lncurses
CFLAGS += -I$(ROOTDIR)/lib/libncurses/include
 
EXEC = pacman 
EXEC2 = pacmanedit
OBJS = pacman.o
OBJS2 = pacmanedit.o
 
.PHONY: all romfs clean
 
all: $(EXEC) $(EXEC2)
 
$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
 
$(EXEC2): $(OBJS2)
	$(CC) $(LDFLAGS) -o $@ $(OBJS2) $(LIBS)
 
romfs: 
	$(ROMFSINST) /usr/games/$(EXEC)
	$(ROMFSINST) /usr/games/$(EXEC2)
	$(ROMFSINST) -d Levels/level01.dat /usr/share/games/pacman/Levels/level01.dat
	$(ROMFSINST) -d Levels/level02.dat /usr/share/games/pacman/Levels/level02.dat
	$(ROMFSINST) -d Levels/level03.dat /usr/share/games/pacman/Levels/level03.dat
	$(ROMFSINST) -d Levels/level04.dat /usr/share/games/pacman/Levels/level04.dat
	$(ROMFSINST) -d Levels/level05.dat /usr/share/games/pacman/Levels/level05.dat
	$(ROMFSINST) -d Levels/level06.dat /usr/share/games/pacman/Levels/level06.dat
	$(ROMFSINST) -d Levels/level07.dat /usr/share/games/pacman/Levels/level07.dat
	$(ROMFSINST) -d Levels/level08.dat /usr/share/games/pacman/Levels/level08.dat
	$(ROMFSINST) -d Levels/level09.dat /usr/share/games/pacman/Levels/level09.dat
	$(ROMFSINST) -d Levels/README /usr/share/games/pacman/Levels/README
	$(ROMFSINST) -d Levels/template.dat /usr/share/games/pacman/Levels/template.dat

clean:
	rm -f $(EXEC) $(EXEC2) *.elf *.gdb *.o
